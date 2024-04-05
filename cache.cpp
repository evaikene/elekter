#include "cache.h"
#include "common.h"

#include <QDir>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringLiteral>
#include <QVariant>

#include <QtSql/qsqldatabase.h>
#include <QtSql/qsqlquery.h>
#include <fmt/format.h>

namespace {
constexpr char const *CACHE_DIR = ".local/share/elekter";
constexpr char const *DB_NAME   = "nordpool.db";

constexpr char const *CREATE_TABLES[] = {

    R"(CREATE TABLE IF NOT EXISTS blocks (
    id INTEGER PRIMARY KEY,
    region CHAR(2) NOT NULL,
    start_h INTEGER NOT NULL,
    size INTEGER NOT NULL))",

    "CREATE UNIQUE INDEX IF NOT EXISTS idx_blocks ON blocks (region, start_h)",

    R"(CREATE TABLE IF NOT EXISTS prices (
    block_id INTEGER NOT NULL,
    time_h INTEGER NOT NULL,
    price DOUBLE NOT NULL))",

    "CREATE UNIQUE INDEX IF NOT EXISTS idx_price_blocks ON prices (block_id, time_h)",

    nullptr

};

constexpr char const *INSERT_BLOCK = "INSERT INTO blocks (region, start_h, size) VALUES (?,?,?)";
constexpr char const *INSERT_PRICE = "INSERT INTO prices (block_id, time_h, price) VALUES (?,?,?)";
constexpr char const *GET_PRICE_BLOCKS =

    R"(SELECT id, start_h, size FROM blocks
    WHERE region = :region AND
          (start_h >= :start OR (start_h + size) > :start) AND
          (start_h <= :end))";

constexpr char const *GET_PRICES =

    R"(SELECT time_h, price FROM prices
        WHERE block_id=:block_id AND time_h >= :start AND time_h <= :end)";

class Transaction {
public:
    inline Transaction(QSqlDatabase &db)
        : _db(&db)
    {
        _db->transaction();
    }

    inline ~Transaction()
    {
        if (_db)
            _db->rollback();
    }

    inline bool commit()
    {
        auto rval = _db->commit();
        if (rval)
            _db = nullptr;
        return rval;
    }

private:
    QSqlDatabase *_db = nullptr;
};

} // namespace

namespace El {

// -----------------------------------------------------------------------------

Cache::Cache(App const &app, QObject *parent)
    : QObject(parent)
    , _app(app)
{
    // create the cache directory
    auto const home = QDir::home();
    if (!home.mkpath(CACHE_DIR)) {
        fmt::print(stderr, "Vahemälu kausta {} loomine ebaõnnestus\n", CACHE_DIR);
        return;
    }

    // initialize the database
    if (!init_database()) {
        return;
    }

    _valid = true;
}

Cache::~Cache() = default;

bool Cache::init_database()
{
    auto db = QSqlDatabase::addDatabase("QSQLITE");

    // open the database
    auto const db_name = QString{"%1/%2/%3"}.arg(QDir::homePath(), CACHE_DIR, DB_NAME);
    db.setDatabaseName(db_name);
    if (!db.open()) {
        fmt::print(stderr, "Vahemälu andmebaasi faili {} avamine ebaõnnestus: {}\n", db_name, db.lastError().text());
        return false;
    }

    // create tables
    QSqlQuery q{db};

    auto sql = CREATE_TABLES;
    for (; *sql; ++sql) {
        if (!q.prepare(*sql)) {
            fmt::print(stderr, "Päringu {} ettevalmistamine ebaõnnestus: {}\n", q.lastQuery(), q.lastError().text());
            return false;
        }
        if (!q.exec()) {
            fmt::print(stderr, "Päringu {} käivitamine ebaõnnestus: {}\n", q.lastQuery(), q.lastError().text());
            return false;
        }
    }

    return true;
}

auto Cache::get_prices(QString const &region, int start_h, int end_h) const -> PriceBlocks const
{
    auto db = QSqlDatabase::database();
    if (!db.isOpen()) {
        throw Exception{"andmebaas ei ole avatud"};
    }

    // prepare SQL statements
    QSqlQuery q_blocks{db};
    if (!q_blocks.prepare(GET_PRICE_BLOCKS)) {
        throw Exception{"päringu {} ettevalmistamine ebaõnnestus: {}", q_blocks.lastQuery(), q_blocks.lastError().text()};
    }

    q_blocks.bindValue(":region", region);
    q_blocks.bindValue(":start", QVariant{start_h});
    q_blocks.bindValue(":end", QVariant{end_h});

    QSqlQuery q_prices{db};
    if (!q_prices.prepare(GET_PRICES)) {
        throw Exception{"päringu {} ettevalmistamine ebaõnnestus: {}", q_prices.lastQuery(), q_prices.lastError().text()};
    }

    q_prices.bindValue(":start", QVariant{start_h});
    q_prices.bindValue(":end", QVariant{end_h});

    if (!q_blocks.exec()) {
        throw Exception{"päringu {} käivitamine ebaõnnestus: {}", q_blocks.lastQuery(), q_blocks.lastError().text()};
    }

    // we now have zero, one or multiple price blocks
    PriceBlocks blocks{};
    while (q_blocks.next()) {

        // load all the prices from this block that are within the request time frame
        q_prices.bindValue(":block_id", q_blocks.value(0).toLongLong());

        if (!q_prices.exec()) {
            throw Exception{"päringu {} käivitamine ebaõnnestus: {}", q_prices.lastQuery(), q_prices.lastError().text()};
        }

        // now we have price records
        PriceBlock block{};
        while (q_prices.next()) {
            block.append({q_prices.value(0).toInt(), q_prices.value(1).toDouble()});
        }

        if (!block.empty()) {
            blocks.append(std::move(block));
        }

    }

    return blocks;
}

void Cache::store_prices(QString const &region, PriceBlocks const &prices)
{
    auto db = QSqlDatabase::database();
    if (!db.isOpen()) {
        throw Exception{"andmebaas ei ole avatud"};
    }

    // prepare SQL statements
    QSqlQuery q_block{db};
    if (!q_block.prepare(INSERT_BLOCK)) {
        throw Exception{"päringu {} ettevalmistamine ebaõnnestus: {}", q_block.lastQuery(), q_block.lastError().text()};
    }
    q_block.bindValue(0, region);

    QSqlQuery q_price{db};
    if (!q_price.prepare(INSERT_PRICE)) {
        throw Exception{"päringu {} ettevalmistamine ebaõnnestus: {}", q_price.lastQuery(), q_price.lastError().text()};
    }

    Transaction tr{db};

    // store all the blocks and prices
    for (auto const &b : prices.blocks()) {
        auto time_h = b.start_time_h;
        q_block.bindValue(1, QVariant{b.start_time_h});
        q_block.bindValue(2, QVariant{b.size()});

        if (!q_block.exec()) {
            throw Exception{"päringu {} käivitamine ebaõnnestus: {}", q_block.lastQuery(), q_block.lastError().text()};
        }

        q_price.bindValue(0, q_block.lastInsertId());

        for (auto const price : b.prices) {
            q_price.bindValue(1, QVariant{time_h++});
            q_price.bindValue(2, QVariant{price});

            if (!q_price.exec()) {
                throw Exception{"päringu {} käivitamine ebaõnnestus: {}", q_price.lastQuery(), q_price.lastError().text()};
            }
        }
    }

    if (!tr.commit()) {
        throw Exception{"andmebaasi salvestamine ebaõnnestus: {}", db.lastError().text()};
    }
}

} // namespace El

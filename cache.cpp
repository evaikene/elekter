#include "cache.h"
#include "common.h"

#include <QDateTime>
#include <QDir>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

#include <fmt/format.h>

#include <array>

namespace {

constexpr auto const *CACHE_DIR = ".local/share/elekter";
constexpr auto const *DB_NAME   = "nordpool.db";

constexpr std::array<char const *, 4> CREATE_TABLES = {

    R"(CREATE TABLE IF NOT EXISTS blocks (
    id INTEGER PRIMARY KEY,
    region CHAR(2) NOT NULL,
    start_s INTEGER NOT NULL,
    end_s INTEGER NOT NULL))",

    "CREATE INDEX IF NOT EXISTS idx_blocks ON blocks (region)",

    R"(CREATE TABLE IF NOT EXISTS prices (
    block_id INTEGER NOT NULL,
    time_s INTEGER NOT NULL,
    price DOUBLE NOT NULL))",

    "CREATE INDEX IF NOT EXISTS idx_price_blocks ON prices (block_id)"
};

constexpr auto const *INSERT_BLOCK = "INSERT INTO blocks (region, start_s, end_s) VALUES (?,?,?)";
constexpr auto const *INSERT_PRICE = "INSERT INTO prices (block_id, time_s, price) VALUES (?,?,?)";
constexpr auto const *GET_PRICE_BLOCKS =

    R"(SELECT id, start_s, end_s FROM blocks
    WHERE region = :region AND
          start_s >= :start AND
          end_s <= :end)";

constexpr auto const *GET_PRICES =

    R"(SELECT time_s, price FROM prices
        WHERE block_id=:block_id AND time_s >= :start AND time_s <= :end)";

class Transaction {
public:
    Transaction(QSqlDatabase &db)
        : _db(&db)
    {
        _db->transaction();
    }

    ~Transaction()
    {
        if (_db != nullptr) {
            _db->rollback();
        }
    }

    auto commit() -> bool
    {
        auto const rval = _db->commit();
        if (rval) {
            _db = nullptr;
        }
        return rval;
    }

private:
    QSqlDatabase *_db = nullptr;
};

} // namespace

namespace El {

// -----------------------------------------------------------------------------

Cache::Cache(App const &app)
    : _app(app)
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

auto Cache::init_database() -> bool
{
    using namespace Qt::Literals::StringLiterals;

    auto db = QSqlDatabase::addDatabase(u"QSQLITE"_s);

    // open the database
    auto const db_name = QString{u"%1/%2/%3"_s}.arg(QDir::homePath(), CACHE_DIR, DB_NAME);
    db.setDatabaseName(db_name);
    if (!db.open()) {
        fmt::print(stderr, "Vahemälu andmebaasi faili {} avamine ebaõnnestus: {}\n", db_name, db.lastError().text());
        return false;
    }

    // create tables
    QSqlQuery q{db};

    for (auto const *sql : CREATE_TABLES) {
        if (!q.prepare(sql)) {
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

auto Cache::get_prices(QString const &region, QDateTime const &start, QDateTime const &end) const -> PriceBlocks
{
    using namespace Qt::Literals::StringLiterals;

    if (!_valid) {
        throw Exception{"vahemälu ei ole avatud"};
    }

    if (end < start) {
        return {};
    }

    auto db = QSqlDatabase::database();
    if (!db.isOpen()) {
        throw Exception{"andmebaas ei ole avatud"};
    }

    // prepare SQL statements
    QSqlQuery q_blocks{db};
    if (!q_blocks.prepare(GET_PRICE_BLOCKS)) {
        throw Exception{"päringu {} ettevalmistamine ebaõnnestus: {}", q_blocks.lastQuery(), q_blocks.lastError().text()};
    }

    q_blocks.bindValue(u":region"_s, region);
    q_blocks.bindValue(u":start"_s, QVariant{start.toSecsSinceEpoch()});
    q_blocks.bindValue(u":end"_s, QVariant{end.toSecsSinceEpoch()});

    QSqlQuery q_prices{db};
    if (!q_prices.prepare(GET_PRICES)) {
        throw Exception{"päringu {} ettevalmistamine ebaõnnestus: {}", q_prices.lastQuery(), q_prices.lastError().text()};
    }

    q_prices.bindValue(u":start"_s, QVariant{start.toSecsSinceEpoch()});
    q_prices.bindValue(u":end"_s, QVariant{end.toSecsSinceEpoch()});

    if (!q_blocks.exec()) {
        throw Exception{"päringu {} käivitamine ebaõnnestus: {}", q_blocks.lastQuery(), q_blocks.lastError().text()};
    }

    // we now have zero, one or multiple price blocks
    PriceBlocks blocks{};
    while (q_blocks.next()) {

        // load all the prices from this block that are within the request time frame
        q_prices.bindValue(u":block_id"_s, q_blocks.value(0).toLongLong());

        if (!q_prices.exec()) {
            throw Exception{"päringu {} käivitamine ebaõnnestus: {}", q_prices.lastQuery(), q_prices.lastError().text()};
        }

        // now we have price records
        PriceBlock block{};
        while (q_prices.next()) {
            block.append({QDateTime::fromSecsSinceEpoch(q_prices.value(0).toLongLong()), q_prices.value(1).toDouble()});
        }

        if (!block.empty()) {
            blocks.append(std::move(block));
        }

    }

    return blocks;
}

void Cache::store_prices(QString const &region, PriceBlocks const &prices) const
{
    if (!_valid) {
        throw Exception{"vahemälu ei ole avatud"};
    }

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
        q_block.bindValue(1, QVariant{b.start_time.toSecsSinceEpoch()});
        q_block.bindValue(2, QVariant{b.end_time.toSecsSinceEpoch()});

        if (!q_block.exec()) {
            throw Exception{"päringu {} käivitamine ebaõnnestus: {}", q_block.lastQuery(), q_block.lastError().text()};
        }

        q_price.bindValue(0, q_block.lastInsertId());

        for (auto const &price : b.prices) {
            q_price.bindValue(1, QVariant{price.time.toSecsSinceEpoch()});
            q_price.bindValue(2, QVariant{price.price});

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

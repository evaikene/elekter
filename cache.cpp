#include "cache.h"
#include "common.h"

#include <QDir>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QStringLiteral>
#include <QVariant>

#include <fmt/format.h>

namespace {
    constexpr char const *CACHE_DIR = ".local/share/elekter";
    constexpr char const *DB_NAME   = "nordpool.db";

    constexpr char const *CREATE_BLOCKS_TABLE =

R"(CREATE TABLE IF NOT EXISTS blocks (
    start_h INTEGER PRIMARY KEY,
    size INTEGER NOT NULL))";

    constexpr char const *CREATE_PRICES_TABLE =

R"(CREATE TABLE IF NOT EXISTS prices (
    time_h INTEGER PRIMARY KEY,
    price DOUBLE NOT NULL))";

    constexpr char const *INSERT_BLOCK = "INSERT INTO blocks (start_h, size) VALUES (?,?)";
    constexpr char const *INSERT_PRICE = "INSERT INTO prices (time_h, price) VALUES (?,?)";

    class Transaction {
    public:
        inline Transaction(QSqlDatabase &db)
            : _db(&db)
        {
            _db->transaction();
        }

        inline ~Transaction()
        {
            if (_db) _db->rollback();
        }

        inline bool commit()
        {
            auto rval = _db->commit();
            if (rval) _db = nullptr;
            return rval;
        }

    private:
        QSqlDatabase *_db = nullptr;
    };

}

namespace El {

// -----------------------------------------------------------------------------

Cache::Cache(App const &app, QObject *parent)
    : QObject(parent)
    , _app(app)
{
    // create the cache directory
    auto const home = QDir::home();
    if (!home.mkpath(CACHE_DIR)) {
        fmt::print(stderr, "Failed to create cache directory {}\n", CACHE_DIR);
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
        fmt::print(stderr, "Failed to open database file {}: {}\n", db_name, db.lastError().text());
        return false;
    }

    // create tables
    QSqlQuery q{db};

    if (!q.prepare(CREATE_BLOCKS_TABLE)) {
        fmt::print(stderr, "Failed to prepare {}: {}\n", q.lastQuery(), q.lastError().text());
        return false;
    }
    if (!q.exec()) {
        fmt::print(stderr, "Failed to exec {}: {}\n", q.lastQuery(), q.lastError().text());
        return false;
    }

    if (!q.prepare(CREATE_PRICES_TABLE)) {
        fmt::print(stderr, "Failed to prepare {}: {}\n", q.lastQuery(), q.lastError().text());
        return false;
    }
    if (!q.exec()) {
        fmt::print(stderr, "Failed to exec {}: {}\n", q.lastQuery(), q.lastError().text());
        return false;
    }

    return true;
}

auto Cache::get_prices(QDateTime const &start, QDateTime const &end) const -> PriceBlocks const
{
    return {};
}

void Cache::store_prices(PriceBlocks const &prices)
{
    fmt::print("Storing prices...\n");
    auto db = QSqlDatabase::database();
    if (!db.isOpen()) {
        throw Exception{"Database is not opened"};
    }

    // prepare SQL statements
    QSqlQuery q_block{db};
    if (!q_block.prepare(INSERT_BLOCK)) {
        throw Exception{fmt::format("Failed to prepare {}: {}\n", q_block.lastQuery(), q_block.lastError().text())};
    }
    QSqlQuery q_price{db};
    if (!q_price.prepare(INSERT_PRICE)) {
        throw Exception{fmt::format("Failed to prepare {}: {}\n", q_price.lastQuery(), q_price.lastError().text())};
    }

    Transaction tr{db};

    // store all the blocks and prices
    for (auto const &b : prices) {
        auto time_h = b.start_time_h;
        q_block.bindValue(0, QVariant{b.start_time_h});
        q_block.bindValue(1, QVariant{b.size()});

        if (!q_block.exec()) {
            throw Exception{fmt::format("Failed to exec {}: {}\n", q_block.lastQuery(), q_block.lastError().text())};
        }

        for (auto const price : b.prices) {
            q_price.bindValue(0, QVariant{time_h++});
            q_price.bindValue(1, QVariant{price});

            if (!q_price.exec()) {
                throw Exception{fmt::format("Failed to exec {}: {}\n", q_price.lastQuery(), q_price.lastError().text())};
            }
        }
    }

    if (!tr.commit()) {
        throw Exception{fmt::format("Failed to commit database changes: {}", db.lastError().text())};
    }

    fmt::print("done.\n");
}

}

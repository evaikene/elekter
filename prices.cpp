#include "prices.h"
#include "args.h"
#include "common.h"

#include <QByteArray>
#include <QString>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

namespace El {

Prices::Prices(Args const & args)
    : _args(args)
{}

bool Prices::loadFromFile(QString const & filename)
{
    // Open the input file
    QFile file(filename);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        fprintf(stderr, "Failed to open prices JSON file \"%s\": %s\n",
                qPrintable(filename),
                qPrintable(file.errorString()));
        return false;
    }

    // Load the file
    auto const json = file.readAll();
    if (json.isEmpty()) {
        fprintf(stderr, "Empty JSON document \"%s\"\n", qPrintable(filename));
        return false;
    }

    // Load prices
    if (!loadFromJson(json)) {
        fprintf(stderr, "Failed to process JSON document \"%s\"\n", qPrintable(filename));
        return false;
    }

    return true;
}

bool Prices::loadFromJson(QByteArray const & json)
{
    auto const doc = QJsonDocument::fromJson(json);

    if (!doc.isObject()) {
        fprintf(stderr, "Invalid JSON document\n%s\n", json.constData());
        return false;
    }
    auto const obj = doc.object();

    // Check for success
    auto const success = obj.value("success");
    if (!success.isBool()) {
        fprintf(stderr, "Invalid \"success\" element\n");
        return false;
    }
    if (!success.toBool()) {
        fprintf(stderr, "The JSON document is not good (\"success\" is false)\n");
        return false;
    }

    // Get data
    auto const data = obj.value("data");
    if (!data.isObject()) {
        fprintf(stderr, "Invalid \"data\" element\n");
        return false;
    }
    auto const ee = data.toObject().value(_args.region());
    if (!ee.isArray()) {
        fprintf(stderr, "Invalid region \"%s\" element\n", _args.region().constData());
        return false;
    }
    auto const prices = ee.toArray();
    auto it = prices.cbegin();
    for (; it != prices.cend(); ++it) {
        if (!it->isObject()) {
            fprintf(stderr, "Invalid price element\n");
            return false;
        }
        auto const p = it->toObject();

        try {
            auto price = Price::from_json(p);
            _prices.insert(price.time_h, price.price);
        }
        catch (Exception const &ex) {
            fprintf(stderr, "%s\n", ex.what());
            return false;
        }
    }

    _valid = true;

    return true;
}

std::optional<double> Prices::getPrice(QDateTime const & time) const
{
    auto const time_h = time.toSecsSinceEpoch() / (60 * 60);
    auto const it = _prices.constFind(time_h);
    if (it == _prices.constEnd()) {
        return std::optional<double>{};
    }
    return std::optional<double>{it.value() / 1000.0};
}

} // namespace El

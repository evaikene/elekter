#include "prices.h"

#include <QByteArray>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>


bool Prices::loadFromFile(QByteArray const & filename)
{
    // Open the input file
    QFile file(filename);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        fprintf(stderr, "Failed to open file \"%s\": %s\n",
                filename.constData(),
                qPrintable(file.errorString()));
        return false;
    }

    // Load the file
    auto const json = QJsonDocument::fromJson(file.readAll());
    if (json.isEmpty()) {
        fprintf(stderr, "Empty JSON document \"%s\"\n", filename.constData());
        return false;
    }
    if (!json.isObject()) {
        fprintf(stderr, "Invalid JSON document \"%s\"\n", filename.constData());
        return false;
    }
    auto const obj = json.object();

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
    auto const ee = data.toObject().value("ee");
    if (!ee.isArray()) {
        fprintf(stderr, "Invalid \"ee\" element\n");
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
        auto const t = p.value("timestamp");
        if (!t.isDouble()) {
            fprintf(stderr, "Invalid \"timestamp\" element \"%s\"\n", qPrintable(t.toString()));
            return false;
        }
        auto const v = p.value("price");
        if (!v.isDouble()) {
            fprintf(stderr, "Invalid \"price\" element \"%s\"\n", qPrintable(v.toString()));
            return false;
        }

        auto const price = v.toDouble();
        _prices.insert(static_cast<qint64>(t.toDouble()), price);
    }

    _valid = true;

    return true;
}

std::optional<double> Prices::getPrice(QDateTime const & time) const
{
    auto const it = _prices.constFind(time.toSecsSinceEpoch());
    if (it == _prices.constEnd()) {
        return std::optional<double>{};
    }
    return std::optional<double>{it.value() / 1000.0};
}

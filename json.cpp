#include "json.h"

#include "args.h"

#include <QByteArray>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

#include <fmt/format.h>

namespace El {

// -----------------------------------------------------------------------------

auto Json::from_json(QByteArray const &json, QString const &region, QDateTime const &end) -> Json
{

    Json me{};
    me.parse(region, json, end);
    return me;
}

// -----------------------------------------------------------------------------

Json::Json() = default;

void Json::parse(QString const &region, QByteArray const &json, QDateTime const &end)
{
    using namespace Qt::Literals::StringLiterals;

    // get the JSON object
    auto const doc = QJsonDocument::fromJson(json);
    if (!doc.isObject()) {
        throw Exception{"Invalid JSON document"};
    }
    auto const obj = doc.object();

    // check for success
    auto const success = obj.value(u"success"_s);
    if (!success.isBool()) {
        throw Exception{"Invalid or missing 'success' element"};
    }
    if (!success.toBool(false)) {
        throw Exception{"The JSON document is not good ('success' element is false)"};
    }

    // get data
    auto const data = obj.value(u"data"_s);
    if (!data.isObject()) {
        throw Exception{"Invalid or missing 'data' element"};
    }

    // get prices for the region
    auto const reg = data.toObject().value(region);
    if (!reg.isArray()) {
        throw Exception{fmt::format("Invalid or missing region '{}' element", region)};
    }
    auto const prices = reg.toArray();

    // parse price records and store them in price blocks
    auto const &args = Args::instance();
    PriceBlock block{};
    QDateTime last_time;
    double last_price = 0.0;
    for (auto const &el : prices) {
        if (!el.isObject()) {
            throw Exception{fmt::format("Invalid price element '{}'", el.toString())};
        }
        auto const o = el.toObject();
        auto price = Price::from_json(o);

        // check for 1 hour intervals that Nord Pool is returning for prices before 2025-10-01
        constexpr int SEC_IN_HOUR = 3'600;
        if (!last_time.isNull() && last_time.addSecs(SEC_IN_HOUR) == price.time) {

            // we have a full hour, so fill in missing 15 minute intervals with the previous price
            for (int i = 0; i < 3; ++i) {
                last_time = last_time.addSecs(args.interval());
                block.append({last_time, last_price});
            }
        }
        last_time = price.time;
        last_price = price.price;

        // check for holes in the block
        if (!block.empty() && block.end_time.addSecs(args.interval()) < price.time) {
            // move the block to the price blocks array
            _prices.append(std::move(block));

            // block is now empty
        }

        block.append(price);
    }

    // fill in missing prices up to 'end' time
    if (!last_time.isNull()) {
        while (last_time < end) {
            last_time = last_time.addSecs(args.interval());
            block.append({last_time, last_price});
        }
    }

    // append the block if it has prices
    if (!block.empty()) {
        _prices.append(std::move(block));
    }
}

} // namespace El

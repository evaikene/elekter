#include "json.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLatin1String>

#include <fmt/format.h>

namespace El {

// -----------------------------------------------------------------------------

Json Json::from_json(QByteArray const &json, QString const &region)
{

    Json me{};
    me.parse(json, region);
    return me;
}

// -----------------------------------------------------------------------------

Json::Json() = default;

Json::~Json() = default;

void Json::parse(QByteArray const &json, QString const &region)
{
    // get the JSON object
    auto const doc = QJsonDocument::fromJson(json);
    if (!doc.isObject()) {
        throw Exception{"Invalid JSON document"};
    }
    auto const obj = doc.object();

    // check for success
    auto const success = obj.value(QLatin1String{"success"});
    if (!success.isBool()) {
        throw Exception{"Invalid or missing 'success' element"};
    }
    if (!success.toBool(false)) {
        throw Exception{"The JSON document is not good ('success' element is false)"};
    }

    // get data
    auto const data = obj.value(QLatin1String{"data"});
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
    PriceBlock *current_block = nullptr;
    for (auto const &el : prices) {
        if (!el.isObject()) {
            throw Exception{fmt::format("Invalid price element '{}'", el.toString())};
        }
        auto const o = el.toObject();
        auto price = Price::from_json(o);

        // check for the current price block
        if (current_block && ((current_block->start_time_h + current_block->size()) != price.time_h)) {
            current_block = nullptr;
        }

        if (!current_block) {
            _prices.push_back(PriceBlock{price.time_h});
            current_block = &_prices.back();
        }

        current_block->prices.push_back(price.price);
    }

    // sort price blocks by start time
    sort(_prices);
}

} // namespace El

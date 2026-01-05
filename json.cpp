#include "json.h"

#include "args.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <fmt/format.h>

namespace El {

// -----------------------------------------------------------------------------

auto Json::from_json(QByteArray const &json, QString const &region) -> Json
{

    Json me{};
    me.parse(region, json);
    return me;
}

// -----------------------------------------------------------------------------

Json::Json() = default;

void Json::parse(QString const &region, QByteArray const &json)
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
    PriceBlock block{};
    for (auto const &el : prices) {
        if (!el.isObject()) {
            throw Exception{fmt::format("Invalid price element '{}'", el.toString())};
        }
        auto const o = el.toObject();
        auto price = Price::from_json(o);

        // check for holes in the block
        if (!block.empty() && block.end_time.addSecs(Args::instance().interval()) < price.time) {
            // move the block to the price blocks array
            _prices.append(std::move(block));

            // block is now empty
        }

        block.append(price);
    }

    // append the block if it has prices
    if (!block.empty()) {
        _prices.append(std::move(block));
    }
}

} // namespace El

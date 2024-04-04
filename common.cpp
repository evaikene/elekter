#include "common.h"

#include <QJsonObject>
#include <QLatin1String>
#include <QVariant>

namespace El {

// -----------------------------------------------------------------------------

auto Price::from_json(QJsonObject const &json) -> Price
{
    if (!json.contains(QLatin1String{"timestamp"})) {
        throw Exception{"Missing 'timestamp' element"};
    }
    auto const t = json.value(QLatin1String{"timestamp"});

    bool ok = false;
    auto const timestamp = t.toVariant().toLongLong(&ok);
    if (!ok) {
        throw Exception{fmt::format("Invalid 'timestamp' element value '{}'", t.toString())};
    }

    if (!json.contains(QLatin1String{"price"})) {
        throw Exception{"Missing 'price' element"};
    }
    auto const p = json.value(QLatin1String{"price"});

    ok = false;
    auto const price = p.toVariant().toDouble(&ok);
    if (!ok) {
        throw Exception{fmt::format("Invalid 'price' element value '{}'", p.toString())};
    }

    return Price{static_cast<int>(timestamp / (60 * 60)), price};
}

// -----------------------------------------------------------------------------

void PriceBlocks::merge(PriceBlocks const &other)
{
    // for all the price blocks in the `other` array
    for (auto const &b: other.blocks()) {
        if (b.empty())
            continue;

        auto const start = b.start_time_h;
        auto const end = b.start_time_h + b.size() - 1;

        
    }
}

} // namespace El

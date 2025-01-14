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

    return Price{static_cast<int>(timestamp / SECS_IN_HOUR), price};
}

} // namespace El

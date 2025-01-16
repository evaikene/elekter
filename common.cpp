#include "common.h"

#include <QJsonObject>
#include <QVariant>

#include <fmt/format.h>

namespace El {

// -----------------------------------------------------------------------------

auto Price::from_json(QJsonObject const &json) -> Price
{
    using namespace Qt::Literals::StringLiterals;

    if (!json.contains(u"timestamp"_s)) {
        throw Exception{"Missing 'timestamp' element"};
    }
    auto const t = json.value(u"timestamp"_s);

    bool ok = false;
    auto const timestamp = t.toVariant().toLongLong(&ok);
    if (!ok) {
        throw Exception{fmt::format("Invalid 'timestamp' element value '{}'", t.toString())};
    }

    if (!json.contains(u"price"_s)) {
        throw Exception{"Missing 'price' element"};
    }
    auto const p = json.value(u"price"_s);

    ok = false;
    auto const price = p.toVariant().toDouble(&ok);
    if (!ok) {
        throw Exception{fmt::format("Invalid 'price' element value '{}'", p.toString())};
    }

    return Price{static_cast<int>(timestamp / SECS_IN_HOUR), price};
}

} // namespace El

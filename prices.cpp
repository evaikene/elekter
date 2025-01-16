#include "prices.h"
#include "app.h"
#include "args.h"
#include "cache.h"
#include "common.h"
#include "nordpool.h"

#include <QDateTime>

#include <fmt/base.h>

namespace El {

Prices::Prices(App const &app)
    : _app(app)
    , _cache(new Cache{app})
{}

Prices::~Prices() = default;

auto Prices::load(QString const &region, QDateTime const &start, QDateTime const &end) -> bool
{
    auto const start_h = to_hours(start);
    auto const end_h   = to_hours(end);

    // try cached prices first
    try {
        _prices = _cache->get_prices(region, start_h, end_h);
    }
    catch (Exception const &ex) {
        fmt::print("WARNING: hindade pärimine vahemälust ebaõnnestu: {}\n", ex.what());
    }

    // check the result
    if (!_prices.empty() && !_prices.has_holes() && start_h >= _prices.start_time_h() && end_h <= _prices.end_time_h()) {
        if (_app.args().verbose()) {
            fmt::print("Kasutan vahemälusse salvestatud hindasid\n");
        }
        return true;

    }

    // request missing prices from Nord Pool
    auto const missing_blocks = _prices.get_missing_blocks(start_h, end_h);

    NordPool np{_app};

    for (auto const &period : missing_blocks) {
        PriceBlocks p;
        try {
            p = np.get_prices(region, period.start_h, period.end_h);
        }
        catch (Exception const &ex) {
            fmt::print(stderr, "ERROR: Nord Pool hindade küsimine ebaõnnestus: {}\n", ex.what());
            return false;
        }

        // update cache
        try {
            _cache->store_prices(region, p);
        }
        catch (Exception const &ex) {
            fmt::print("WARNING: Nord Pool hindade salvestamine vahemälusse ebaõnnestus: {}\n", ex.what());
        }

        _prices.append(std::move(p));
    }

    return true;
}

auto Prices::get_price(QDateTime const &time) const -> std::optional<double>
{
    auto const value = _prices.get_price(to_hours(time));
    if (!value) {
        return {};
    }

    return *value / KWH_IN_MWH;
}

} // namespace El

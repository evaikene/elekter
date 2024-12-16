#pragma once

#ifndef EL_PRICES_H_INCLUDED
#  define EL_PRICES_H_INCLUDED

#include "common.h"

#include <QtGlobal>

#include <memory>
#include <optional>

QT_FORWARD_DECLARE_CLASS(QDateTime)
QT_FORWARD_DECLARE_CLASS(QString)

namespace El {

class App;
class Cache;

/// Hourly Nord Pool prices
class Prices {
public:

    /// Ctor
    /// @param[in] app Application instance
    Prices(App const &app);

    /// Dtor
    ~Prices();

    /// Loads hourly prices for the given time period
    /// @param[in] region Price region
    /// @param[in] start Start time
    /// @param[in] end End time
    /// @return true when succeeded, otherwise false
    auto load(QString const &region, QDateTime const &start, QDateTime const &end) -> bool;

    /// Get the price in Euros for one kWh for the given time
    /// @param[in] time The date/time
    /// @return The price or an empty value
    auto get_price(QDateTime const &time) const -> std::optional<double>;

private:

    /// Application instance
    App const &_app;

    /// Prices cache
    std::unique_ptr<Cache> _cache;

    /// Price blocks
    PriceBlocks _prices;
};

} // namespace El

#endif

#pragma once

#ifndef EL_PRICES_H
#  define EL_PRICES_H

#include "common.h"

#include <QObject>

#include <optional>

QT_BEGIN_NAMESPACE
    class QDateTime;
    class QString;
QT_END_NAMESPACE

namespace El {

class App;
class Cache;

/// Hourly Nord Pool prices
class Prices : public QObject {
    Q_OBJECT

public:

    /// Ctor
    /// @param[in] app Application instance
    /// @param[in] parent Optional parent
    Prices(App const &app, QObject *parent = nullptr);

    /// Dtor
    ~Prices() override = default;

    /// Loads hourly prices for the given time period
    /// @param[in] region Price region
    /// @param[in] start Start time
    /// @param[in] end End time
    /// @return true when succeeded, otherwise false
    bool load(QString const &region, QDateTime const &start, QDateTime const &end);

    /// Get the price in Euros for one kWh for the given time
    /// @param[in] time The date/time
    /// @return The price or an empty value
    auto get_price(QDateTime const &time) const -> std::optional<double>;

private:

    /// Application instance
    App const &_app;

    /// Prices cache
    Cache *_cache = nullptr;

    /// Price blocks
    PriceBlocks _prices;
};

} // namespace El

#endif

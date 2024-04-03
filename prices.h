#pragma once

#ifndef EL_PRICES_H
#  define EL_PRICES_H

#include <QMap>
#include <QVector>

#include <optional>

class QDateTime;
class QByteArray;
class QString;

namespace El {

class Args;

/// Hourly Nordpool prices
class Prices {
public:

    /// Ctor
    Prices(Args const &args);

    /// Dtor
    ~Prices() = default;

    /// Returns true if prices are valid
    inline bool valid() const noexcept { return _valid; }

    /// Load hourly prices from the JSON file
    /// @param[in] filename Name of the file
    /// @return True when succeeded, otherwise false
    bool loadFromFile(QString const &filename);

    /// Load hourly prices from the JSON document
    /// @param[in] json The JSON document
    /// @return True when succeeded, otherwise false
    bool loadFromJson(QByteArray const &json);

    /// Get the price in Euros for one kWh for the given time
    /// @param[in] time The date/time
    /// @return The price or an empty value
    std::optional<double> getPrice(QDateTime const &time) const;

private:

    /// Arguments
    Args const &_args;

    /// Flag indicating that prices are valid
    bool _valid = false;

    /// time/price (EUR/MWh) pairs (price per MWh)
    QMap<qint64, double> _prices;
};

} // namespace El

#endif

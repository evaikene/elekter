#pragma once

#ifndef EL_CACHE_H
#  define EL_CACHE_H

#include "common.h"

#include <QObject>
#include <QString>

namespace El {

class App;

/// Nord Pool price history cache
class Cache : public QObject {
    Q_OBJECT

public:

    /// Ctor
    Cache(App const &app, QObject *parent = nullptr);

    /// Dtor
    ~Cache() override;

    /// Returns true if the cache is valid and can be used
    inline bool valid() const noexcept { return _valid; }

    /// Retrieves Nord Pool prices from the cache
    /// @param[in] region Price region
    /// @param[in] start_h Start time  (hours since the EPOCH)
    /// @param[in] end_h End time (hours since the EPOCH)
    /// @return Price blocks with Nord Pool prices (may contain holes)
    /// @throws El::Exception on errors
    auto get_prices(QString const &region, int start_h, int end_h) const -> PriceBlocks const;

    /// Stores Nord Pool prices
    /// @param[in] region Price region
    /// @param[in] prices Price blocks
    /// @throws El::Exception on errors
    void store_prices(QString const &region, PriceBlocks const &prices);

private:

    /// Application instance
    App const &_app;

    /// Flag indicating that cache is valid and can be used
    bool _valid = false;

    /// Initializes the cache database
    bool init_database();

};

} // namespace El

#endif

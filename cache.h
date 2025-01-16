#pragma once

#ifndef EL_CACHE_H_INCLUDED
#  define EL_CACHE_H_INCLUDED

#include "common.h"

#include <QObject>
#include <QString>

namespace El {

class App;

/// Nord Pool price history cache
class Cache {
public:

    /// Ctor
    Cache(App const &app);

    /// Dtor
    ~Cache() = default;

    /// Returns true if the cache is valid and can be used
    inline auto valid() const noexcept -> auto { return _valid; }

    /// Retrieves Nord Pool prices from the cache
    /// @param[in] region Price region
    /// @param[in] start_h Start time  (hours since the EPOCH)
    /// @param[in] end_h End time (hours since the EPOCH)
    /// @return Price blocks with Nord Pool prices (may contain holes)
    /// @throws El::Exception on errors
    auto get_prices(QString const &region, int start_h, int end_h) const -> PriceBlocks;

    /// Stores Nord Pool prices
    /// @param[in] region Price region
    /// @param[in] prices Price blocks
    /// @throws El::Exception on errors
    void store_prices(QString const &region, PriceBlocks const &prices) const;

private:

    /// Application instance
    App const &_app;

    /// Flag indicating that cache is valid and can be used
    bool _valid = false;

    /// Initializes the cache database
    static auto init_database() -> bool;

};

} // namespace El

#endif

#pragma once

#ifndef EL_COMMON_H
#  define EL_COMMON_H

#include <QByteArray>
#include <QDateTime>
#include <QString>
#include <QVector>

#include <fmt/format.h>

#include <algorithm>
#include <stdexcept>
#include <string>

QT_BEGIN_NAMESPACE
    class QJsonObject;
QT_END_NAMESPACE

template <>
struct fmt::formatter<QString>
{
    template <typename ParseContext>
    constexpr auto parse(ParseContext &ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(QString const &v, FormatContext &ctx)
    {
        return fmt::format_to(ctx.out(), "{}", qPrintable(v));
    }
};

template <>
struct fmt::formatter<QByteArray>
{
    template <typename ParseContext>
    constexpr auto parse(ParseContext &ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(QByteArray const &v, FormatContext &ctx)
    {
        return fmt::format_to(ctx.out(), "{}", v.constData());
    }
};

template <>
struct fmt::formatter<QDateTime>
{
    template <typename ParseContext>
    constexpr auto parse(ParseContext &ctx)
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(QDateTime const &v, FormatContext &ctx)
    {
        return fmt::format_to(ctx.out(), "{}", v.toString());
    }
};

namespace El {

/// Exception
class Exception : public std::runtime_error {
public:

    /// ctor
    inline Exception(std::string const &msg)
        : std::runtime_error(msg)
    {}

};

/// Nord Pool hourly price record
struct Price {

    /// Constructs the Price record from a JSON object
    /// @param[in] json JSON object with `timestamp` and `price` attributes
    /// @return Price record
    /// @throws Exception on errors
    static auto from_json(QJsonObject const &json) -> Price;

    /// Ctor
    inline Price(int time_h_, double price_)
        : time_h(time_h_)
        , price(price_)
    {}

    /// Time as hours since the EPOCH
    int time_h = 0;

    /// Price (EUR/MWh) without taxes
    double price = 0.0;
};

/// Nord Pool hourly price block with start time and length in number of hours
struct PriceBlock {

    /// Constructs the price block with the given starting time
    inline explicit PriceBlock(int time_h)
        : start_time_h(time_h)
    {}

    /// Size of the block in number of hours
    auto size() const { return prices.size(); }

    /// Start time as hours since the EPOCH
    int start_time_h = 0;

    /// Hourly prices (EUR/MHh) without taxes
    QVector<double> prices;
};

/// Array of price blocks that MUST always be sorted by the start time
using PriceBlocks = QVector<PriceBlock>;

/// Sorts the array of price blocks by the start time
inline void sort(PriceBlocks &blocks)
{
    std::sort(blocks.begin(), blocks.end(), [](PriceBlock const &a, PriceBlock const &b) {
        return a.start_time_h < b.start_time_h;
    });
}

/// Checks for holes in price blocks
/// @param[in] blocks Price blocks
/// @return true if there are holes, otherwise false
///
/// Price blocks MUST be sorted by the start time prior calling this function
inline bool has_holes(PriceBlocks const &blocks)
{
    if (blocks.isEmpty()) return false;

    auto start = blocks.first().start_time_h;
    auto size = blocks.first().size();
    auto it = blocks.cbegin();
    for (++it; it != blocks.cend(); ++it) {
        if (it->start_time_h > (start + size)) {
            // hole detected
            return true;
        }
        start = it->start_time_h;
        size = it->size();
    }

    // no holes detected
    return false;
}

} // namespace El

#endif

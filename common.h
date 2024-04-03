#pragma once

#ifndef COMMON_H
#  define COMMON_H

#include <QDateTime>
#include <QString>
#include <QVector>

#include <fmt/format.h>

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

/// NordPool hourly price record
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

/// NordPool hourly price block with start time and length in number of hours
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

using PriceBlocks = QVector<PriceBlock>;

} // namespace El

#endif

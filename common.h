#pragma once

#ifndef EL_COMMON_H_INCLUDED
#  define EL_COMMON_H_INCLUDED

#include "args.h"

#include <QByteArray>
#include <QDateTime>
#include <QString>
#include <QVector>

#include <fmt/format.h>

#include <algorithm>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

QT_FORWARD_DECLARE_CLASS(QJsonObject)

template <>
struct fmt::formatter<QByteArray> : public fmt::formatter<std::string_view> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext &ctx)
    {
        return fmt::formatter<std::string_view>::parse(ctx);
    }

    template <typename FormatContext>
    auto format(QByteArray const &v, FormatContext &ctx) const
    {
        auto const sv = std::string_view{v.constData(), static_cast<size_t>(v.size())};
        return fmt::formatter<std::string_view>::format(sv, ctx);
    }
};

template <>
struct fmt::formatter<QString> : public fmt::formatter<QByteArray> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext &ctx)
    {
        return fmt::formatter<QByteArray>::parse(ctx);
    }

    template <typename FormatContext>
    auto format(QString const &v, FormatContext &ctx) const
    {
        return fmt::formatter<QByteArray>::format(v.toUtf8(), ctx);
    }
};

template <>
struct fmt::formatter<QDateTime> : public fmt::formatter<QString> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext &ctx)
    {
        return fmt::formatter<QString>::parse(ctx);
    }

    template <typename FormatContext>
    auto format(QDateTime const &v, FormatContext &ctx) const
    {
        return fmt::formatter<QString>::format(v.toString("yyyy-MM-dd hh:mm"), ctx);
    }
};

namespace El {

/// Number of kWh in a MWh
constexpr double KWH_IN_MWH = 1000.0;

/// Exception
class Exception : public std::runtime_error {
public:

    /// ctor
    Exception(std::string const &msg)
        : std::runtime_error(msg)
    {}

    template <typename... Args>
    Exception(std::string const &fmt, Args const &...args)
        : std::runtime_error(fmt::format(fmt, args...))
    {}
};

/// Returns the number of hours since the EPOCH
/// @param[in] dt Date/time
/// @return Number of hours since the EPOCH
static inline auto to_hours(QDateTime const &dt) -> int
{
    constexpr qint64 SECS_IN_MIN  = 60;
    constexpr qint64 MINS_IN_HOUR = 60;
    return static_cast<int>(dt.toSecsSinceEpoch() / (SECS_IN_MIN * MINS_IN_HOUR));
}

/// Returns the date/time value from the number of hours since the EPOCH
/// @param[in] time_h Number of hours since the EPOCH
/// @return Date/time value
static inline auto to_datetime(int time_h) -> QDateTime
{
    constexpr qint64 SECS_IN_MIN  = 60;
    constexpr qint64 MINS_IN_HOUR = 60;
    return QDateTime::fromSecsSinceEpoch(static_cast<qint64>(time_h) * SECS_IN_MIN * MINS_IN_HOUR);
}

/// Nord Pool hourly price record
struct Price {

    /// Constructs the Price record from a JSON object
    /// @param[in] json JSON object with `timestamp` and `price` attributes
    /// @return Price record
    /// @throws Exception on errors
    static auto from_json(QJsonObject const &json) -> Price;

    /// Ctor
    Price(QDateTime time_, double price_)
        : time(std::move(time_))
        , price(price_)
    {}

    /// Dtor
    ~Price() = default;

    /// Time of the price
    QDateTime time;

    /// Price (EUR/MWh) without taxes
    double price = 0.0;
};

/// Nord Pool price block with start and end time
struct PriceBlock {

    /// Default ctr
    PriceBlock() = default;

    /// Dtir
    ~PriceBlock() = default;

    /// Move and copy operations
    PriceBlock(PriceBlock const &rhs) = default;

    PriceBlock(PriceBlock &&rhs) noexcept
    {
        start_time.swap(rhs.start_time);
        end_time.swap(rhs.end_time);
        prices.swap(rhs.prices);
    }

    auto operator=(PriceBlock const &rhs) -> PriceBlock &
    {
        if (this != &rhs) {
            start_time = rhs.start_time;
            end_time   = rhs.end_time;
            prices     = rhs.prices;
        }
        return *this;
    }

    auto operator=(PriceBlock &&rhs) noexcept -> PriceBlock &
    {
        if (this != &rhs) {
            start_time = QDateTime{};
            start_time.swap(rhs.start_time);
            end_time  = QDateTime{};
            end_time.swap(rhs.end_time);
            prices.clear();
            prices.swap(rhs.prices);
        }
        return *this;
    }

    /// Returns true if the block is empty
    auto empty() const { return prices.isEmpty(); }

    /// Appends a price to the block
    void append(Price const &price)
    {
        if (prices.isEmpty()) {
            start_time = price.time;
            end_time   = price.time;
        }
        prices.append(price);

        bool sort = false;
        if (start_time > price.time) {
            start_time = price.time;
            sort       = true;
        }
        if (end_time > price.time) {
            sort = true;
        }
        else {
            end_time = price.time;
        }

        if (sort) {
            std::sort(prices.begin(), prices.end(), [](Price const &a, Price const &b) {
                return a.time < b.time;
            });
        }
    }

    auto get_price(QDateTime const &time) const -> std::optional<double>
    {
        // iterate over prices
        for (auto it = prices.cbegin(); it != prices.cend(); ++it) {
            // check for exact match
            if (it->time == time) {
                return it->price;
            }

            // check if we have passed the time
            if (it->time > time) {
                // it is the previous price (if any)
                if (it != prices.cbegin()) {
                    return (it - 1)->price;
                }

                // no previous price
                return {};
            }
        }
        return {};
    }

    /// Start time of the block
    QDateTime start_time;

    /// End time of the block
    QDateTime end_time;

    /// Hourly prices (EUR/MHh) without taxes
    QVector<Price> prices;
};

/// Start and end time pair
struct TimePair {
    QDateTime start; ///< Start time
    QDateTime end;   ///< End time
};

/// Array of price blocks that is always sorted by the start time
class PriceBlocks {
public:

    /// Ctor
    PriceBlocks() = default;

    /// Dtor
    ~PriceBlocks() = default;

    /// Copy and move operations
    PriceBlocks(PriceBlocks const &)                         = default;
    PriceBlocks(PriceBlocks &&) noexcept                     = default;
    auto operator=(PriceBlocks const &) -> PriceBlocks     & = default;
    auto operator=(PriceBlocks &&) noexcept -> PriceBlocks & = default;

    /// Returns the number of price blocks
    auto size() const { return _blocks.size(); }

    /// Returns true if the array of price blocks is empty
    auto empty() const { return _blocks.isEmpty(); }

    /// Returns the array of price blocks
    auto blocks() const noexcept -> auto const & { return _blocks; }

    /// Returns the last price block in the array
    auto last() -> auto & { return _blocks.back(); }

    auto last() const -> auto const & { return _blocks.back(); }

    /// Returns the start time
    auto start_time() const -> QDateTime
    {
        if (empty()) {
            return {};
        }

        return _blocks.first().start_time;
    }

    /// Returns the end time
    auto end_time() const -> QDateTime
    {
        if (empty()) {
            return {};
        }

        return _blocks.last().end_time;
    }

    /// Appends a price block
    /// @param[in] block Price block being added
    void append(PriceBlock const &block)
    {
        _blocks.append(block);
        sort();
        normalize();
    }

    /// Appends a price block
    /// @param[in] block Price block being added
    void append(PriceBlock &&block)
    {
        _blocks.append(std::forward<PriceBlock>(block));
        sort();
        normalize();
    }

    /// Appends prices from another prices array
    /// @param[in] blocks Other prices array
    void append(PriceBlocks const &blocks)
    {
        _blocks.append(blocks._blocks);
        sort();
        normalize();
    }

    /// Appends prices from another prices array
    /// @param[in] blocks Other prices array
    void append(PriceBlocks &&blocks)
    {
        _blocks.append(std::forward<QVector<PriceBlock>>(blocks._blocks));
        sort();
        normalize();
    }

    /// Checks for holes in price blocks
    /// @return true if there are holes, otherwise false
    auto has_holes() const -> bool
    {
        if (_blocks.isEmpty()) {
            return false;
        }

        auto end   = _blocks.first().end_time;
        auto it    = _blocks.cbegin() + 1;
        while (it != _blocks.cend()) {
            if (it->start_time > end) {
                // hole detected
                return true;
            }
            end   = it->end_time;
            ++it;
        }

        // no holes detected
        return false;
    }

    /// Returns missing price blocks information
    /// @param[in] start Expected start time
    /// @param[in] end Expected end time
    /// @return Array of holes
    auto get_missing_blocks(QDateTime const &start, QDateTime const &end) const -> QVector<TimePair>
    {
        if (empty()) {
            return {
                {start, end}
            };
        }

        QVector<TimePair> result{};

        // check for missing prices before the first block
        if (start < start_time()) {
            result.append({start, start_time().addSecs(-1)});
        }

        // checks for holes between price blocks
        auto const &args = Args::instance();
        auto bend  = _blocks.first().end_time;
        auto it    = _blocks.cbegin() + 1;
        while (it != _blocks.cend()) {

            // check for the end time
            if (end <= bend) {
                return result;
            }

            // check for missing data between price blocks
            if (it->start_time > bend.addSecs(args.interval())) {
                // hole detected
                result.append({bend, it->start_time.addSecs(-1)});
            }

            bend = it->end_time;

            ++it;
        }

        // check for missing prices after the last block
        if (end > end_time()) {
            result.append({end_time().addSecs(1), end});
        }

        return result;
    }

    /// Returns price for the given time
    /// @param[in] time Time value
    /// @return Price as EUR/MWh when succeeded, otherwise an invalid optional
    auto get_price(QDateTime const &time) const -> std::optional<double>
    {
        // find the block that contains the given time
        auto it = std::find_if(_blocks.cbegin(), _blocks.cend(), [&time](PriceBlock const &b) {
            return time >= b.start_time && time <= b.end_time;
        });
        if (it == _blocks.cend()) {
            return {};
        }

        // find the price within the block
        return it->get_price(time);
    }

private:

    /// Array of price blocks
    QVector<PriceBlock> _blocks;

    /// Sorts the array of price blocks by the start time
    void sort()
    {
        std::sort(_blocks.begin(), _blocks.end(), [](PriceBlock const &a, PriceBlock const &b) {
            return a.start_time < b.start_time;
        });
    }

    /// Normalizes the array by merging individual blocks without holes
    /// The price blocks array MUST be sorted before calling this function
    void normalize()
    {
        if (_blocks.isEmpty()) {
            return;
        }

        QVector<PriceBlock> normalized;
        for (auto const &b : _blocks) {
            if (normalized.isEmpty()) {
                normalized.append(b);
            }
            else {
                if (b.start_time > normalized.back().end_time.addSecs(1)) {
                    // there is a hole
                    normalized.append(b);
                }
                else {
                    // block continues
                    normalized.back().prices.append(b.prices);
                }
            }
        }

        _blocks = std::move(normalized);
    }

    /// Finds a block that contains the given time
    /// @param[in] time Time to find
    /// @return Pointer to the prices block or NULL if not found
    auto find_block(QDateTime const &time) -> PriceBlock *
    {
        auto it = std::find_if(_blocks.begin(), _blocks.end(), [&time](PriceBlock const &b) {
            return time >= b.start_time && time <= b.end_time;
        });
        if (it != _blocks.end()) {
            return &*it;
        }

        return nullptr;
    }
};

} // namespace El

#endif // EL_COMMON_H_INCLUDED

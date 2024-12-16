#pragma once

#ifndef EL_COMMON_H_INCLUDED
#  define EL_COMMON_H_INCLUDED

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
struct fmt::formatter<QString> : public fmt::formatter<std::string_view> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext &ctx)
    {
        return fmt::formatter<std::string_view>::parse(ctx);
    }

    template <typename FormatContext>
    auto format(QString const &v, FormatContext &ctx) const
    {
        return fmt::formatter<std::string_view>::format(v.toStdString(), ctx);
    }
};

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
        return fmt::formatter<std::string_view>::format(v.toStdString(), ctx);
    }
};

template <>
struct fmt::formatter<QDateTime> : public fmt::formatter<std::string_view> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext &ctx)
    {
        return fmt::formatter<std::string_view>::parse(ctx);
    }

    template <typename FormatContext>
    auto format(QDateTime const &v, FormatContext &ctx) const
    {
        return fmt::formatter<std::string_view>::format(v.toString("yyyy-MM-dd hh:mm").toStdString(), ctx);
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

    template <typename... Args>
    Exception(std::string const &fmt, Args const &...args)
        : std::runtime_error(fmt::format(fmt, args...))
    {}
};

/// Returns the number of hours since the EPOCH
/// @param[in] dt Date/time
/// @return Number of hours since the EPOCH
inline auto to_hours(QDateTime const &dt) -> int
{
    static constexpr qint64 SECS_IN_MIN  = 60;
    static constexpr qint64 MINS_IN_HOUR = 60;
    return static_cast<int>(dt.toSecsSinceEpoch() / (SECS_IN_MIN * MINS_IN_HOUR));
}

/// Returns the date/time value from the number of hours since the EPOCH
/// @param[in] time_h Number of hours since the EPOCH
/// @return Date/time value
inline auto to_datetime(int time_h) -> QDateTime
{
    static constexpr qint64 SECS_IN_MIN  = 60;
    static constexpr qint64 MINS_IN_HOUR = 60;
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
    inline Price(int time_h_, double price_)
        : time_h(time_h_)
        , price(price_)
    {}

    /// Dtor
    ~Price() = default;

    /// Time as hours since the EPOCH
    int time_h = 0;

    /// Price (EUR/MWh) without taxes
    double price = 0.0;
};

/// Nord Pool hourly price block with start time and length in number of hours
struct PriceBlock {

    /// Default ctr
    PriceBlock() = default;

    /// Dtir
    ~PriceBlock() = default;

    /// Move and copy operations
    PriceBlock(PriceBlock const &rhs) = default;

    inline PriceBlock(PriceBlock &&rhs) noexcept
    {
        std::swap(start_time_h, rhs.start_time_h);
        std::swap(prices, rhs.prices);
    }

    inline auto operator=(PriceBlock const &rhs) -> PriceBlock &
    {
        if (this != &rhs) {
            start_time_h = rhs.start_time_h;
            prices       = rhs.prices;
        }
        return *this;
    }

    inline auto operator=(PriceBlock &&rhs) noexcept -> PriceBlock &
    {
        if (this != &rhs) {
            start_time_h     = rhs.start_time_h;
            rhs.start_time_h = 0;
            prices           = std::move(rhs.prices);
        }
        return *this;
    }

    /// Size of the block in number of hours
    auto size() const { return prices.size(); }

    /// Returns true if the block is empty
    auto empty() const { return prices.isEmpty(); }

    /// Appends a price to the block
    inline void append(Price const &price)
    {
        if (prices.isEmpty()) {
            start_time_h = price.time_h;
        }
        prices.append(price.price);
    }

    /// Start time as hours since the EPOCH
    int start_time_h = 0;

    /// Hourly prices (EUR/MHh) without taxes
    QVector<double> prices;
};

/// Start and end time pair
struct TimePair {
    int start_h = 0; ///< Start time as number of hours since the EPOCH
    int end_h   = 0; ///< End time as number of hours since the EPOCH
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
    inline auto size() const { return _blocks.size(); }

    /// Returns true if the array of price blocks is empty
    inline auto empty() const { return _blocks.isEmpty(); }

    /// Returns the array of price blocks
    inline auto blocks() const noexcept -> auto const & { return _blocks; }

    /// Returns the last price block in the array
    inline auto last() -> auto & { return _blocks.back(); }

    inline auto last() const -> auto const & { return _blocks.back(); }

    /// Returns the start time (hours since the EPOCH)
    inline auto start_time_h() const -> int
    {
        if (empty()) {
            return 0;
        }

        return _blocks.first().start_time_h;
    }

    /// Returns the end time (hours since the EPOCH)
    inline auto end_time_h() const -> int
    {
        if (empty()) {
            return 0;
        }

        return _blocks.last().start_time_h + _blocks.last().size() - 1;
    }

    /// Appends a price block
    /// @param[in] block Price block being added
    inline void append(PriceBlock const &block)
    {
        _blocks.append(block);
        sort();
        normalize();
    }

    /// Appends a price block
    /// @param[in] block Price block being added
    inline void append(PriceBlock &&block)
    {
        _blocks.append(std::forward<PriceBlock>(block));
        sort();
        normalize();
    }

    /// Appends prices from another prices array
    /// @param[in] blocks Other prices array
    inline void append(PriceBlocks const &blocks)
    {
        _blocks.append(blocks._blocks);
        sort();
        normalize();
    }

    /// Appends prices from another prices array
    /// @param[in] blocks Other prices array
    inline void append(PriceBlocks &&blocks)
    {
        _blocks.append(std::forward<QVector<PriceBlock>>(blocks._blocks));
        sort();
        normalize();
    }

    /// Checks for holes in price blocks
    /// @return true if there are holes, otherwise false
    inline auto has_holes() const -> bool
    {
        if (_blocks.isEmpty()) {
            return false;
        }

        auto        start = _blocks.first().start_time_h;
        auto        size  = _blocks.first().size();
        auto const *it    = _blocks.cbegin();
        for (++it; it != _blocks.cend(); ++it) {
            if (it->start_time_h > (start + size)) {
                // hole detected
                return true;
            }
            start = it->start_time_h;
            size  = it->size();
        }

        // no holes detected
        return false;
    }

    /// Returns missing price blocks information
    /// @param[in] start_h Expected start time in hours since the EPOCH
    /// @param[in] end_h Expected end time in hours since the EPOCH
    /// @return Array of holes
    inline auto get_missing_blocks(int start_h, int end_h) const -> QVector<TimePair>
    {
        if (empty()) {
            return {
                {start_h, end_h}
            };
        }

        QVector<TimePair> result{};

        // check for missing prices before the first block
        if (start_h < start_time_h()) {
            result.append({start_h, start_time_h() - 1});
        }

        // checks for holes between price blocks
        auto start = _blocks.first().start_time_h;
        auto size  = _blocks.first().size();
        auto const *it    = _blocks.cbegin();
        for (++it; it != _blocks.cend(); ++it) {

            // check for the end time
            if (end_h <= (start + size)) {
                return result;
            }

            // check for missing data between price blocks
            if (it->start_time_h > (start + size)) {
                // hole detected
                result.append({start + size, it->start_time_h - 1});
            }

            start = it->start_time_h;
            size  = it->size();
        }

        // check for missing prices after the last block
        if (end_h > end_time_h()) {
            result.append({end_time_h() + 1, end_h});
        }

        return result;
    }

    /// Checks for overlapping blocks
    /// @param[in] block Block being tested
    /// @return True if the block overlaps with existing blocks, false if not
    inline auto is_overlapping(PriceBlock const &block) const -> bool
    {
        if (_blocks.isEmpty() || block.empty()) {
            return false;
        }

        auto const start = block.start_time_h;
        auto const end   = start + block.size() - 1;

        if (std::any_of(_blocks.constBegin(), _blocks.constEnd(), [start, end](PriceBlock const &b) {
            return (start >= b.start_time_h && start < (b.start_time_h + b.size())) ||
                   (end >= b.start_time_h && end < (b.start_time_h + b.size()));
        })) {
            return true;
        };

        // not overlapping
        return false;
    }

    /// Returns price for the given time
    /// @param[in] time_h Time value as hours since the EPOCH
    /// @return Price as EUR/MWh when succeeded, otherwise an invalid optional
    inline auto get_price(int time_h) const -> std::optional<double>
    {
        auto const *it = std::find_if(_blocks.cbegin(), _blocks.cend(), [time_h](PriceBlock const &b) {
            return time_h >= b.start_time_h && time_h < (b.start_time_h + b.size());
        });
        if (it == _blocks.cend()) {
            return {};
        }
        return it->prices[time_h - it->start_time_h];
    }

private:

    /// Array of price blocks
    QVector<PriceBlock> _blocks;

    /// Sorts the array of price blocks by the start time
    inline void sort()
    {
        std::sort(_blocks.begin(), _blocks.end(), [](PriceBlock const &a, PriceBlock const &b) {
            return a.start_time_h < b.start_time_h;
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
                if (b.start_time_h > (normalized.back().start_time_h + normalized.back().size())) {
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

    /// Finds a block that contains the given time (hours since the EPOCH)
    /// @param[in] time_h Time to find
    /// @return Pointer to the prices block or NULL if not found
    inline auto find_block(int time_h) -> PriceBlock *
    {
        auto *it = std::find_if(_blocks.begin(), _blocks.end(), [time_h](PriceBlock const &b) {
            return time_h >= b.start_time_h && time_h < (b.start_time_h + b.size());
        });
        if (it != _blocks.end()) {
            return it;
        }

        return nullptr;
    }
};

} // namespace El

#endif

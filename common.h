#pragma once

#ifndef EL_COMMON_H
#  define EL_COMMON_H

#  include <QByteArray>
#  include <QDateTime>
#  include <QString>
#  include <QVector>

#  include <fmt/format.h>

#  include <algorithm>
#  include <optional>
#  include <stdexcept>
#  include <string>

QT_BEGIN_NAMESPACE
    class QJsonObject;
QT_END_NAMESPACE

template <>
struct fmt::formatter<QString> {
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
struct fmt::formatter<QByteArray> {
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
struct fmt::formatter<QDateTime> {
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

/// Returns the number of hours since the EPOCH
/// @param[in] dt Date/time
/// @return Number of hours since the EPOCH
inline int to_hours(QDateTime const dt)
{
    return dt.toSecsSinceEpoch() / (60 * 60);
}

/// Returns the date/time value from the number of hours since the EPOCH
/// @param[in] time_h Number of hours since the EPOCH
/// @return Date/time value
inline auto to_datetime(int time_h) -> QDateTime
{
    return QDateTime::fromSecsSinceEpoch(time_h * 60 * 60);
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

    /// Time as hours since the EPOCH
    int time_h = 0;

    /// Price (EUR/MWh) without taxes
    double price = 0.0;
};

/// Nord Pool hourly price block with start time and length in number of hours
struct PriceBlock {

    /// Default ctr
    PriceBlock() = default;

    /// Move and copy operations
    inline PriceBlock(PriceBlock const &rhs)
        : start_time_h(rhs.start_time_h)
        , prices(rhs.prices)
    {}

    inline PriceBlock(PriceBlock &&rhs) noexcept
    {
        std::swap(start_time_h, rhs.start_time_h);
        std::swap(prices, rhs.prices);
    }

    inline PriceBlock &operator=(PriceBlock const &rhs)
    {
        if (this != &rhs) {
            start_time_h = rhs.start_time_h;
            prices       = rhs.prices;
        }
        return *this;
    }

    inline PriceBlock &operator=(PriceBlock &&rhs)
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
    PriceBlocks(PriceBlocks const &)                = default;
    PriceBlocks(PriceBlocks &&) noexcept            = default;
    PriceBlocks &operator=(PriceBlocks const &)     = default;
    PriceBlocks &operator=(PriceBlocks &&) noexcept = default;

    /// Returns the number of price blocks
    inline auto size() const { return _blocks.size(); }

    /// Returns true if the array of price blocks is empty
    inline auto empty() const { return _blocks.isEmpty(); }

    /// Returns the array of price blocks
    inline auto const &blocks() const noexcept { return _blocks; }

    /// Returns the last price block in the array
    inline auto &last() { return _blocks.back(); }

    inline auto const &last() const { return _blocks.back(); }

    /// Returns the start time (hours since the EPOCH)
    inline int start_time_h() const
    {
        if (empty())
            return 0;

        return _blocks.first().start_time_h;
    }

    /// Returns the end time (hours since the EPOCH)
    inline int end_time_h() const
    {
        if (empty())
            return 0;

        return _blocks.last().start_time_h + _blocks.last().size() - 1;
    }

    /// Merges with other price blocks array
    /// @param[in] other Other price blocks array
    ///
    /// Adds prices from `other` that are not in this price blocks array
    void merge(PriceBlocks const &other);

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

    /// Checks for holes in price blocks
    /// @return true if there are holes, otherwise false
    inline bool has_holes() const
    {
        if (_blocks.isEmpty())
            return false;

        auto start = _blocks.first().start_time_h;
        auto size  = _blocks.first().size();
        auto it    = _blocks.cbegin();
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
            return {{start_h, end_h}};
        }

        QVector<TimePair> result{};

        // check for missing prices before the first block
        if (start_h < start_time_h()) {
            result.append({start_h, start_time_h() - 1});
        }

        // checks for holes between price blocks
        auto start = _blocks.first().start_time_h;
        auto size  = _blocks.first().size();
        auto it    = _blocks.cbegin();
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
            result.append({end_h, end_time_h()});
        }

        return result;
    }

    /// Checks for overlapping blocks
    /// @param[in] block Block being tested
    /// @return True if the block overlaps with existing blocks, false if not
    inline bool is_overlapping(PriceBlock const &block) const
    {
        if (_blocks.isEmpty() || block.size() == 0)
            return false;

        auto const start = block.start_time_h;
        auto const end   = start + block.size() - 1;

        for (auto const &b : _blocks) {
            if (start >= b.start_time_h && start < (b.start_time_h + b.size()) ||
                (end >= b.start_time_h && end < (b.start_time_h + b.size()))) {
                return true;
            }
        }

        // not overlapping
        return false;
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
        if (_blocks.isEmpty())
            return;

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
        auto it = std::find_if(_blocks.begin(), _blocks.end(), [time_h](PriceBlock const &b) {
            return time_h >= b.start_time_h && time_h < (b.start_time_h + b.size());
        });
        if (it != _blocks.end())
            return &(*it);

        return nullptr;
    }
};

} // namespace El

#endif

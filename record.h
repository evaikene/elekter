#pragma once

#ifndef EL_RECORD_H_INCLUDED
#  define EL_RECORD_H_INCLUDED

#include <QDateTime>

QT_FORWARD_DECLARE_CLASS(QByteArray)

namespace El {

/// One record from the CSV file
class Record {
public:

    Record(int lineno, QByteArray const &line, bool old = false);
    Record(Record const &other) = default;
    Record(Record &&other)      = default;

    ~Record() = default;

    auto operator=(Record const &other) -> Record & = default;

    /// Returns true if the record is valid
    inline auto isValid() const noexcept { return _valid; }

    /// Returns true if this is night-time record
    inline auto isNight() const noexcept { return _night; }

    /// Returns the start time of the record
    inline auto startTime() const noexcept -> auto const & { return _begin; }

    /// Returns the end time of the record
    inline auto endTime() const noexcept -> auto const & { return _end; }

    /// Returns the amount consumed in this time period in kWh
    inline auto kWh() const noexcept -> auto { return _kWh; }

private:

    bool      _valid = false;
    QDateTime _begin;
    QDateTime _end;
    double    _kWh   = 0.0;
    bool      _night = false;

    /// Processes the input line
    /// @param[in] lineno Line number
    /// @param[in] line   Input line
    /// @param[in] old    True if the old format is used
    /// @returns true if succeeded; false if not
    auto process(int lineno, QByteArray const &line, bool old) -> bool;
};

} // namespace El

#endif

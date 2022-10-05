#pragma once

#ifndef RECORD_H
#define RECORD_H

#include <QDateTime>

class QByteArray;

/// One record from the CSV file
class Record {
public:

    Record(int lineno, QByteArray const & line, bool old = false);
    Record(Record const & other) = default;
    Record(Record && other) = default;

    ~Record() = default;

    Record & operator= (Record const & other) = default;

    /// Returns true if the record is valid
    inline bool isValid() const noexcept { return _valid; }

    /// Returns true if this is night-time record
    inline bool isNight() const noexcept { return _night; }

    /// Returns the start time of the record
    inline QDateTime const & startTime() const noexcept { return _begin; }

    /// Returns the end time of the record
    inline QDateTime const & endTime() const noexcept { return _end; }

    /// Returns the amount consumed in this time period in kWh
    inline double kWh() const noexcept { return _kWh; }


private:

    bool _valid = false;
    QDateTime _begin;
    QDateTime _end;
    double _kWh = 0.0;
    bool _night = false;

    /// Processes the input line
    /// @returns true if succeeded; false if not
    bool process(int lineno, QByteArray const & line, bool old);

};

#endif

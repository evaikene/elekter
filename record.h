#ifndef RECORD_H
#define RECORD_H

#include <QDateTime>

class QByteArray;

/// One record from the CSV file
class Record
{
public:
    Record(QByteArray const & line);
    Record(Record const & other) = default;
    Record(Record && other) = default;

    ~Record() = default;

    Record & operator= (Record const & other) = default;

    /// Returns true if the record is valid
    bool isValid() const { return _valid; }

    /// Returns true if this is night-time record
    bool isNight() const { return _night; }

    /// Returns the start time of the record
    QDateTime const & startTime() const { return _begin; }

    /// Returns the end time of the record
    QDateTime const & endTime() const { return _end; }

    /// Returns the amount consumed in this time period in kWh
    double kWh() const { return _kWh; }

private:

    bool _valid;
    QDateTime _begin;
    QDateTime _end;
    double _kWh;
    bool _night;

    /// Processes the input line
    /// @returns true if succeeded; false if not
    bool process(QByteArray const & line);

};

#endif

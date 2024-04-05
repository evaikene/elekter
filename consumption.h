#pragma once

#ifndef EL_CONSUMPTION_H
#  define EL_CONSUMPTION_H

#include "record.h"

#include <QDateTime>
#include <QObject>
#include <QVector>

namespace El {

class App;

/// Container class for consumption records
class Consumption : public QObject {
    Q_OBJECT

public:

    /// Ctor
    /// @param[in] app Application instance
    /// @param[in] parent Optional parent
    Consumption(App const &app, QObject *parent = nullptr);

    /// Dtor
    ~Consumption() = default;

    /// Loads records from the given CSV file
    /// @param[in] filename Name of the CSV file
    /// @return True when succeeded, otherwise false
    bool load(QString const &filename);

    /// Returns consumption records
    auto const &records() const noexcept { return _records; }

    /// Returns the time of the first record
    auto const &first_record_time() const noexcept { return _first_record_time; }

    /// Returns the time of the last record
    auto const &last_record_time() const noexcept { return _last_record_time; }

private:

    /// Application instance
    App const &_app;

    /// Consumption records
    QVector<Record> _records;

    /// Time of the first consumption record
    QDateTime _first_record_time;

    /// Time of the last consumption record
    QDateTime _last_record_time;

};

} // namespace El

#endif

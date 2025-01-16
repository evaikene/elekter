#pragma once

#ifndef EL_CONSUMPTION_H_INCLUDED
#  define EL_CONSUMPTION_H_INCLUDED

#include "record.h"

#include <QDateTime>
#include <QVector>

namespace El {

class App;

/// Container class for consumption records
class Consumption {
public:

    /// Ctor
    /// @param[in] app Application instance
    Consumption(App const &app);

    /// Dtor
    ~Consumption();

    /// Loads records from the given CSV file
    /// @param[in] filename Name of the CSV file
    /// @return True when succeeded, otherwise false
    auto load(QString const &filename) -> bool;

    /// Returns consumption records
    auto records() const noexcept -> auto const & { return _records; }

    /// Returns the time of the first record
    auto first_record_time() const noexcept -> auto const & { return _first_record_time; }

    /// Returns the time of the last record
    auto last_record_time() const noexcept -> auto const & { return _last_record_time; }

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

#pragma once

#ifndef EL_HEADER_H_INCLUDED
#  define EL_HEADER_H_INCLUDED

#include <QtTypes>

QT_FORWARD_DECLARE_CLASS(QByteArray)

namespace El {

/// CSV file header information
class Header {
public:

    /// Default ctr
    Header() = default;

    /// Constructor
    /// @param[in] line Input line
    Header(QByteArray const &line);

    /// Default move and copy operations
    Header(Header const &other) = default;
    Header(Header &&other)      = default;
    auto operator=(Header const &rhs) -> Header & = default;
    auto operator=(Header &&rhs) -> Header & = default;

    /// Dtor
    ~Header() = default;

    /// Returns true if the header is valid
    auto isValid() const noexcept { return _valid; }

    /// Returns the expected number of fields
    auto numFields() const noexcept { return _num_fields; }

    /// Returns the index of the start time field
    auto idxStartTime() const noexcept { return _idx_start_time; }

    /// Returns the index of the end time field (-1 if not used)
    auto idxEndTime() const noexcept { return _idx_end_time; }

    /// Returns the index of the consumption field
    auto idxConsumption() const noexcept { return _idx_consumption; }

    /// Returns the index of the consumption type field (-1 if not used)
    auto idxConsumptionType() const noexcept { return _idx_consumption_type; }

private:

    /// Validity flag
    bool _valid = false;

    /// Expected number of fields
    qsizetype _num_fields = -1;

    /// Index of the start time field
    qsizetype _idx_start_time = -1;

    /// Index of the end time field
    qsizetype _idx_end_time = -1;

    /// Index of the consumption field
    qsizetype _idx_consumption = -1;

    /// Index of the consumption type field
    qsizetype _idx_consumption_type = -1;

    /// Process the header line and initialize the fields
    /// @param[in] line Input line
    /// @returns true if succeeded; false if not
    auto process(QByteArray const &line) -> bool;

};

} // namespace El

#endif // EL_HEADER_H_INCLUDED

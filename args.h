#pragma once

#ifndef EL_ARGS_H_INCLUDED
#  define EL_ARGS_H_INCLUDED

#include <QByteArray>
#include <QDateTime>
#include <QString>

#include <optional>

namespace El {

class Args {
public:

    Args(int argc, char *argv[]); // NOLINT
    ~Args() = default;

    Args()                                      = delete;
    Args(Args const &other)                     = delete;
    Args(Args &&other)                          = delete;
    auto operator=(Args const &other) -> Args & = delete;

    /// Returns true if arguments are valid
    auto isValid() const noexcept { return _valid; }

    /// Verbose flag
    auto verbose() const noexcept { return _verbose; }

    /// The name of the CSV file to be processed
    auto fileName() const noexcept -> auto const & { return _fileName; }

    /// True if prices are requested
    auto prices() const noexcept { return _prices; }

    /// The name of the JSON file with prices
    auto priceFileName() const noexcept -> auto const & { return _priceFileName; }

    auto region() const noexcept -> auto const & { return _region; }

    /// Margin EUR/kWh
    auto margin() const noexcept { return _margin; }

    /// Returns the initial value for day
    auto startDay() const noexcept { return _day; }

    /// Returns the initial value for night
    auto startNight() const noexcept { return _night; }

    /// Returns the requested date/time
    auto time() const noexcept -> auto const & { return _time; }

    /// Returns the VAT value
    auto km() const noexcept { return _km; }

private:

    static constexpr double DEFAULT_MARGIN = 0.0;

    /// Static instance
    static Args *_instance;

    static void printUsage(bool err, char const *appName);

    bool                  _valid   = false;
    bool                  _verbose = false;
    QString               _fileName;
    bool                  _prices = false;
    QString               _priceFileName;
    QString               _region;
    double                _margin = DEFAULT_MARGIN;
    std::optional<double> _day;
    std::optional<double> _night;
    QDateTime             _time;
    double                _km        = 0.0;
};

} // namespace El

#endif

#pragma once

#ifndef EL_ARGS_H_INCLUDED
#  define EL_ARGS_H_INCLUDED

#include <QByteArray>
#include <QDateTime>
#include <QString>
#include <QStringLiteral>

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
    inline auto isValid() const noexcept { return _valid; }

    /// Verbose flag
    inline auto verbose() const noexcept { return _verbose; }

    /// The name of the CSV file to be processed
    inline auto fileName() const noexcept -> auto const & { return _fileName; }

    /// True if prices are requested
    inline auto prices() const noexcept { return _prices; }

    /// The name of the JSON file with prices
    inline auto priceFileName() const noexcept -> auto const & { return _priceFileName; }

    inline auto region() const noexcept -> auto const & { return _region; }

    /// Margin EUR/kWh
    inline auto margin() const noexcept { return _margin; }

    /// Returns the number skipped files
    inline auto skip() const noexcept { return _skip; }

    /// Returns the initial value for day
    inline auto startDay() const noexcept { return _day; }

    /// Returns the initial value for night
    inline auto startNight() const noexcept { return _night; }

    /// Returns the requested date/time
    inline auto time() const noexcept -> auto const & { return _time; }

    /// Returns true if the input file is in the old format (generated before 2022-03)
    inline auto oldFormat() const noexcept { return _oldFormat; }

    /// Returns the VAT value
    inline auto km() const noexcept { return _km; }

private:

    static constexpr int    DEFAULT_SKIP   = 12;
    static constexpr double DEFAULT_MARGIN = 0.0;

    /// Static instance
    static Args *_instance;

    static void printUsage(bool err, char const *appName);

    bool                  _valid   = false;
    bool                  _verbose = false;
    QString               _fileName;
    bool                  _prices = false;
    QString               _priceFileName;
    QString               _region = QStringLiteral(u"ee");
    double                _margin = DEFAULT_MARGIN;
    int                   _skip   = DEFAULT_SKIP;
    std::optional<double> _day;
    std::optional<double> _night;
    QDateTime             _time;
    bool                  _oldFormat = false;
    double                _km        = 0.0;
};

} // namespace El

#endif

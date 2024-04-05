#pragma once

#ifndef EL_ARGS_H
#  define EL_ARGS_H

#  include <QByteArray>
#  include <QDateTime>
#  include <QString>
#  include <QStringLiteral>

#  include <optional>

namespace El {

class Args {
public:

    static Args const *instance();

    Args(int argc, char *argv[]);
    ~Args();

    Args()                             = delete;
    Args(Args const &other)            = delete;
    Args(Args &&other)                 = delete;
    Args &operator=(Args const &other) = delete;

    /// Returns true if arguments are valid
    inline bool isValid() const noexcept { return _valid; }

    /// Verbose flag
    inline bool verbose() const noexcept { return _verbose; }

    /// The name of the CSV file to be processed
    inline QString const &fileName() const noexcept { return _fileName; }

    /// True if prices are requested
    inline bool prices() const noexcept { return _prices; }

    /// The name of the JSON file with prices
    inline QString const &priceFileName() const noexcept { return _priceFileName; }

    inline QString const &region() const noexcept { return _region; }

    /// Margin EUR/kWh
    inline double const &margin() const noexcept { return _margin; }

    /// Returns the number skipped files
    inline int skip() const noexcept { return _skip; }

    /// Returns the initial value for day
    inline std::optional<double> startDay() const noexcept { return _day; }

    /// Returns the initial value for night
    inline std::optional<double> startNight() const noexcept { return _night; }

    /// Returns the requested date/time
    inline QDateTime const &time() const noexcept { return _time; }

    /// Returns true if the input file is in the old format (generated before 2022-03)
    inline bool oldFormat() const noexcept { return _oldFormat; }

    /// Returns the VAT value
    inline double km() const noexcept { return _km; }

private:

    /// Static instance
    static Args *_instance;

    static void printUsage(bool err, char const *appName);

    bool                  _valid   = false;
    bool                  _verbose = false;
    QString               _fileName;
    bool                  _prices = false;
    QString               _priceFileName;
    QString               _region = QStringLiteral(u"ee");
    double                _margin = 0.0;
    int                   _skip   = 12;
    std::optional<double> _day;
    std::optional<double> _night;
    QDateTime             _time;
    bool                  _oldFormat = false;
    double                _km        = 0.0;
};

} // namespace El

#endif

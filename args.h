#ifndef ARGS_H
#define ARGS_H

#include <QByteArray>
#include <QDateTime>

#include <optional>

class Args
{
public:

    static Args const * instance();

    Args(int argc, char * argv[]);
    ~Args();

    /// Returns true if arguments are valid
    inline bool isValid() const noexcept { return _valid; }

    /// Verbose flag
    inline bool verbose() const noexcept { return _verbose; }

    /// The name of the CSV file to be processed
    inline QByteArray const & fileName() const noexcept { return _fileName; }

    /// The name of the JSON file with prices
    inline QByteArray const & priceFileName() const noexcept { return _priceFileName; }

    inline QByteArray const & region() const noexcept { return _region; }

    /// Margin EUR/kWh
    inline double const & margin() const noexcept { return _margin; }

    /// Returns the number skipped files
    inline int skip() const noexcept { return _skip; }

    /// Returns the initial value for day
    inline std::optional<double> startDay() const noexcept { return _day; }

    /// Returns the initial value for night
    inline std::optional<double> startNight() const noexcept { return _night; }

    /// Returns the requested date/time
    inline QDateTime const & time() const noexcept { return _time; }

private:

    /// Static instance
    static Args * _instance;

    static void printUsage(bool err, char const * appName);

    bool _valid = false;
    bool _verbose = false;
    QByteArray _fileName;
    QByteArray _priceFileName;
    QByteArray _region = "ee";
    double _margin = 0.0;
    int _skip = 6;
    std::optional<double> _day;
    std::optional<double> _night;
    QDateTime _time;

    Args() = delete;
    Args(Args const & other) = delete;
    Args(Args && other) = delete;
    Args & operator= (Args const & other) = delete;
};

#endif

#ifndef ARGS_H
#define ARGS_H

#include <QByteArray>
#include <QDateTime>

class Args
{
public:

    static Args const * instance();

    Args(int argc, char * argv[]);
    ~Args();

    /// Returns true if arguments are valid
    bool isValid() const { return _valid; }

    /// The name of the CSV file to be processed
    QByteArray const & fileName() const { return _fileName; }

    /// Returns the number skipped files
    int skip() const { return _skip; }

    /// Returns the initial value day value
    double startDay() const { return _day; }

    /// Returns the initial value for night value
    double startNight() const { return _night; }

    /// Returns the requested date/time
    QDateTime const & time() const { return _time; }

private:

    /// Static instance
    static Args * _instance;

    static void printUsage(bool err, char const * appName);

    bool _valid;
    QByteArray _fileName;
    int _skip;
    double _day;
    double _night;
    QDateTime _time;

    Args() = delete;
    Args(Args const & other) = delete;
    Args(Args && other) = delete;
    Args & operator= (Args const & other) = delete;
};

#endif

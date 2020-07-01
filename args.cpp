#include "args.h"

#include <stdio.h>
#include <getopt.h>

namespace
{
    char const * const USAGE =
        "USAGE: %1$s [args] <filename>\n"
        "\n"
        "filename:\n"
        "\tThe CSV file to be processed.\n"
        "\n"
        "args:\n"
        "\t-h,--help        Shows this help and exists.\n"
        "\t-d,--day <v>     The initial day value.\n"
        "\t-n,--night <v>   The initial night value.\n"
        "\t-s,--skip <n>    Number of skipped lines (defaults to 6).\n"
        "\t-t,--time <dt>   Requested date/time.\n"
        "\n";

        char const * const shortOpts = "hd:n:s:t:";
        struct option const longOpts[] = {
            { "help",       no_argument,        nullptr, 'h' },
            { "day",        required_argument,  nullptr, 'd' },
            { "night",      required_argument,  nullptr, 'n' },
            { "skip",       required_argument,  nullptr, 's' },
            { "time",       required_argument,  nullptr, 't' },
            { nullptr,      0,                  nullptr, 0 }
        };
}

Args * Args::_instance = nullptr;

Args const * Args::instance()
{
    return _instance;
}

void Args::printUsage(bool err, char const * appName)
{
    fprintf(err ? stderr : stdout, USAGE, appName);
}

Args::Args(int argc, char * argv[])
    : _valid(false)
    , _skip(6)
    , _day(0.0)
    , _night(0.0)
    , _time(QDateTime::currentDateTime())
{
    _instance = this;

    bool daySet = false;
    bool nightSet = false;
    char const * appName = argv[0];
    int c = 0;
    int idx = 0;
    while ((c = getopt_long(argc, argv, shortOpts, longOpts, &idx)) != -1) {
        switch (c) {

            case 'h': {
                printUsage(false, appName);
                return;
            }

            case 'd': {
                char * e = nullptr;
                _day = strtod(optarg, &e);
                if (e == nullptr || *e != '\0') {
                    fprintf(stderr, "Invalid argument \"%s\" for option \'--day\'\n", optarg);
                    printUsage(true, appName);
                    return;
                }
                daySet = true;
                break;
            }

            case 'n': {
                char * e = nullptr;
                _night = strtod(optarg, &e);
                if (e == nullptr || *e != '\0') {
                    fprintf(stderr, "Invalid argument \"%s\" for option \'--night\'\n", optarg);
                    printUsage(true, appName);
                    return;
                }
                nightSet = true;
                break;
            }

            case 's': {
                char * e = nullptr;
                _skip = strtol(optarg, &e, 10);
                if (e == nullptr || *e != '\0') {
                    fprintf(stderr, "Invalid argument \"%s\" for option \'--skip\'\n", optarg);
                    printUsage(true, appName);
                    return;
                }
                break;
            }

            case 't': {
                _time = QDateTime::fromString(optarg, "yyyy-MM-dd hh:mm");
                if (!_time.isValid()) {
                    fprintf(stderr, "Invalid argument \"%s\" for option \'--time\'\n", optarg);
                    printUsage(true, appName);
                    return;
                }
                break;
            }

            case ':': {
                fprintf(stderr, "Missing argument\n\n");
                printUsage(true, appName);
                return;
            }

            default: {
                printUsage(true, appName);
                return;
            }
        }
    }

    // Verify that both day and night start values are given
    if (daySet != nightSet) {
        fprintf(stderr, "Both day and night values shall be given\n");
        return;
    }

    // Verify that the filename is given
    if (optind == argc) {
        fprintf(stderr, "Missing file name\n\n");
        printUsage(true, appName);
        return;
    }
    _fileName = argv[optind++];
    // Verify that only one filename is given
    if (optind != argc) {
        fprintf(stderr, "Only one file name can be provided\n\n");
        return;
    }

    _valid = true;
}

Args::~Args()
{
    _instance = nullptr;
}
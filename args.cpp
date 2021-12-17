#include "args.h"

#include <stdio.h>
#include <getopt.h>

namespace
{
    char const * const USAGE =
        "KASUTAMINE: %1$s [args] <CSV faili nimi>\n"
        "\n"
        "args:\n"
        "\t-h,--help        Näitab seda abiteksti.\n"
        "\t-d,--day <v>     Päevase näidu algväärtus.\n"
        "\t-n,--night <v>   Öise näidu algväärtus.\n"
        "\t-p,--prices <filename> JSON fail Nord Pool tunnihindadega.\n"
        "\t-s,--skip <n>    Faili algusest ignoreeritavate ridade arv (vaikimisi 6).\n"
        "\t-t,--time <dt>   Lõppnäidu kuupäev ja kellaaeg (yyyy-MM-dd hh:mm)\n"
        "\t                 Vaikimisi kasutab praegust aega.\n"
        "\n"
        "Töötleb elektrilevi.ee lehelt allalaaditud CSV-vormingus tunnitarbimise faile.\n"
        "\n"
        "NÄITEKS:\n"
        "\n"
        "Arvuta elektrinäit 20. juuli 2020 kell 9:00 seisuga kasutades andmeid failist 2020-06.csv\n"
        "ning kuu päevast algnäitu 26869.830 ning öist algnäitu 34059.650:\n"
        "> %1$s --day=26869.830 --night=34059.650 --time=\"2020-06-20 09:00\" 2020-06.csv\n"
        "\n"
        "Näita summaarset tarbimist kasutades andmeid failist 2020-06.csv:\n"
        "> %1$s 2020-06.csv\n"
        "\n"
        "Näita summaarset tarbimist kasutades andmeid failist 2020-06.csv ja arvuta\n"
        "elektri eest tasutav summa kasutades hindasid failist 2020-06.json jättes vahele\n"
        "esimesed 4 rida CSV failist:\n"
        "> %1$s -p 2020-06.json 2020-06.csv -s 4\n"
        "\n";

        char const * const shortOpts = "hd:n:p:s:t:";
        struct option const longOpts[] = {
            { "help",       no_argument,        nullptr, 'h' },
            { "day",        required_argument,  nullptr, 'd' },
            { "night",      required_argument,  nullptr, 'n' },
            { "prices",     required_argument,  nullptr, 'p' },
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
    : _time(QDateTime::currentDateTime())
{
    _instance = this;

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

            case 'p': {
                _priceFileName = optarg;
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
    if (_day.has_value() != _night.has_value()) {
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

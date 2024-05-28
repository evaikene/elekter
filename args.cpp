#include "args.h"

#include <fmt/format.h>

#include <getopt.h>

namespace {
constexpr double DEFAULT_VAT = 0.22;

constexpr char const *USAGE = R"(
KASUTAMINE: {0} [args] <CSV faili nimi>

args:
    -h,--help        Näitab seda abiteksti.
    -d,--day <v>     Päevase näidu algväärtus.
    -k[<km%>],--km[=<km%>] Näita hindasid koos käibemaksuga (vaikimisi {1:.0f}%).
    -m,--margin <v>  Elektrimüüja juurdehindlus EUR/kWh;
                     juurdehindlus on koos käibemaksuga, kui --km on antud.
    -n,--night <v>   Öise näidu algväärtus.
    -o,--old         CSV fail on genereeritud enne 2022-03.
                     Faili algusest ignoreeritavate ridade arv on 4.
    -p[<filename>],--prices[=<filename>] Näita hindasid Nord Pool tunnihindadega.
                     Kasutab JSON faili <filename> tunnihindadega või küsib üle võrgu.
    -r,--region <r>  Hinnapiirkond ("ee", "fi", "lv", "lt")
                     vaikimisi kasutab hinnapiirkonda "ee".
    -s,--skip <n>    Faili algusest ignoreeritavate ridade arv (vaikimisi 12 ja 4 vanas formaadis).
    -t,--time <dt>   Lõppnäidu kuupäev ja kellaaeg (yyyy-MM-dd hh:mm)
                     Vaikimisi kasutab praegust aega.
    -v,--verbose     Teeb programmi jutukamaks.

Töötleb elektrilevi.ee lehelt allalaaditud CSV-vormingus tunnitarbimise faile.

NÄITEKS:

Arvuta elektrinäit 20. juuli 2020 kell 9:00 seisuga kasutades andmeid failist 2020-06.csv
ning kuu päevast algnäitu 26869.830 ning öist algnäitu 34059.650:

> {0} --day=26869.830 --night=34059.650 --time="2020-06-20 09:00" 2020-06.csv

Näita summaarset tarbimist kasutades andmeid failist 2020-06.csv:

> {0} 2020-06.csv

Näita summaarset tarbimist kasutades andmeid failist 2020-06.csv ja arvuta elektri
eest tasutav summa koos käibemaksuga kasutades võrgust küsitud hindasid ning elektri-
müüja juurdehindlust 0.45 senti kWh kohta:

> {0} 2020-06.csv -k -p -m 0.0045

Näita summaarset tarbimist kasutades andmeid failist 2020-06.csv ja arvuta
elektri eest tasutav summa koos käibemaksuga kasutades hindasid failist 2020-06.json
jättes vahele esimesed 4 rida CSV failist:

> {0} -k -p2020-06.json 2020-06.csv -s 4
)";

constexpr char const         *shortOpts  = "hd:k::m:n:p::or:s:t:v";
constexpr struct option const longOpts[] = {
    {"help",    no_argument,       nullptr, 'h'},
    {"day",     required_argument, nullptr, 'd'},
    {"km",      optional_argument, nullptr, 'k'},
    {"margin",  required_argument, nullptr, 'm'},
    {"night",   required_argument, nullptr, 'n'},
    {"old",     no_argument,       nullptr, 'o'},
    {"prices",  optional_argument, nullptr, 'p'},
    {"region",  required_argument, nullptr, 'r'},
    {"skip",    required_argument, nullptr, 's'},
    {"time",    required_argument, nullptr, 't'},
    {"verbose", no_argument,       nullptr, 'v'},
    {nullptr,   0,                 nullptr, 0  }
};

} // namespace

namespace El {

Args *Args::_instance = nullptr;

Args const *Args::instance()
{
    return _instance;
}

void Args::printUsage(bool err, char const *appName)
{
    fmt::print(err ? stderr : stdout, USAGE, appName, DEFAULT_VAT * 100.0);
}

Args::Args(int argc, char *argv[])
    : _time(QDateTime::currentDateTime())
{
    _instance = this;

    char const *appName = argv[0];
    int         c       = 0;
    int         idx     = 0;
    while ((c = getopt_long(argc, argv, shortOpts, longOpts, &idx)) != -1) {
        switch (c) {

            case 'h': {
                printUsage(false, appName);
                return;
            }

            case 'd': {
                char *e = nullptr;
                _day    = strtod(optarg, &e);
                if (e == nullptr || *e != '\0') {
                    fmt::print(stderr, "Vigane väärtus \"{}\" argumendile \'--day\'\n", optarg);
                    printUsage(true, appName);
                    return;
                }
                break;
            }

            case 'k': {
                if (optarg) {
                    char *e = nullptr;
                    _km     = strtod(optarg, &e) / 100.0;
                    if (e == nullptr || (*e != '\0' && *e != '%')) {
                        fmt::print(stderr, "Vigane väärtus \"{}\" argumendile \'--km\'\n", optarg);
                        printUsage(true, appName);
                        return;
                    }
                    if (*e == '%' && *(e + 1) != '\0') {
                        fmt::print(stderr, "Vigane väärtus \"{}\" argumendile \'--km\'\n", optarg);
                        printUsage(true, appName);
                        return;
                    }
                }
                else {
                    _km = DEFAULT_VAT;
                }
                break;
            }

            case 'm': {
                char *e = nullptr;
                _margin = strtod(optarg, &e);
                if (e == nullptr || *e != '\0') {
                    fmt::print(stderr, "Vigane väärtus \"{}\" argumendile \'--margin\'\n", optarg);
                    printUsage(true, appName);
                    return;
                }
                break;
            }

            case 'n': {
                char *e = nullptr;
                _night  = strtod(optarg, &e);
                if (e == nullptr || *e != '\0') {
                    fmt::print(stderr, "Vigane väärtus \"{}\" argumendile \'--night\'\n", optarg);
                    printUsage(true, appName);
                    return;
                }
                break;
            }

            case 'o': {
                _oldFormat = true;
                _skip      = 4;
                break;
            }

            case 's': {
                char *e = nullptr;
                _skip   = strtol(optarg, &e, 10);
                if (e == nullptr || *e != '\0') {
                    fmt::print(stderr, "Vigane väärtus \"{}\" argumendile \'--skip\'\n", optarg);
                    printUsage(true, appName);
                    return;
                }
                break;
            }

            case 't': {
                _time = QDateTime::fromString(optarg, "yyyy-MM-dd hh:mm");
                if (!_time.isValid()) {
                    fmt::print(stderr, "Vigane väärtus \"{}\" argumendile \'--time\'\n", optarg);
                    printUsage(true, appName);
                    return;
                }
                break;
            }

            case 'p': {
                _prices = true;
                if (optarg) {
                    _priceFileName = QString::fromUtf8(QByteArray{optarg});
                }
                break;
            }

            case 'r': {
                _region = QString::fromUtf8(optarg);
                break;
            }

            case 'v': {
                _verbose = true;
                break;
            }

            case ':': {
                fmt::print(stderr, "Argumendi väärtus puudub\n\n");
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
        fmt::print(stderr, "Nii päeva kui öö väärtused peavad olema antud\n");
        return;
    }

    // Verify that the filename is given
    if (optind == argc) {
        fmt::print(stderr, "Faili nimi puudub\n\n");
        printUsage(true, appName);
        return;
    }
    _fileName = QString::fromUtf8(QByteArray{argv[optind++]});
    // Verify that only one filename is given
    if (optind != argc) {
        fmt::print(stderr, "Ainut üks failinimi võib olla antud\n\n");
        return;
    }

    _valid = true;
}

Args::~Args()
{
    _instance = nullptr;
}

} // namespace El

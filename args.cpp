#include "args.h"

#include <fmt/base.h>

#include <cstdlib>

#include <getopt.h>

namespace {

constexpr double DEFAULT_VAT = 0.24;

constexpr char const *USAGE = R"(
KASUTAMINE: {0} [args] <CSV faili nimi>

args:
    -h,--help        Näitab seda abiteksti.
    -d,--day <v>     Päevase näidu algväärtus.
    -k[<km%>],--km[=<km%>] Näita hindasid koos käibemaksuga (vaikimisi {1:.0f}%).
    -m,--margin <v>  Elektrimüüja juurdehindlus EUR/kWh;
                     juurdehindlus on koos käibemaksuga, kui --km on antud.
    -n,--night <v>   Öise näidu algväärtus.
    -p[<filename>],--prices[=<filename>] Näita hindasid Nord Pool tunnihindadega.
                     Kasutab JSON faili <filename> tunnihindadega või küsib üle võrgu.
    -r,--region <r>  Hinnapiirkond ("ee", "fi", "lv", "lt")
                     vaikimisi kasutab hinnapiirkonda "ee".
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
elektri eest tasutav summa koos käibemaksuga kasutades hindasid failist 2020-06.json:

> {0} -k -p2020-06.json 2020-06.csv
)";

constexpr char const         *shortOpts  = "hd:k::m:n:p::r:t:v";
constexpr struct option const longOpts[] = { // NOLINT(modernize-avoid-c-arrays)
    {"help",    no_argument,       nullptr, 'h'},
    {"day",     required_argument, nullptr, 'd'},
    {"km",      optional_argument, nullptr, 'k'},
    {"margin",  required_argument, nullptr, 'm'},
    {"night",   required_argument, nullptr, 'n'},
    {"prices",  optional_argument, nullptr, 'p'},
    {"region",  required_argument, nullptr, 'r'},
    {"time",    required_argument, nullptr, 't'},
    {"verbose", no_argument,       nullptr, 'v'},
    {nullptr,   0,                 nullptr, 0  }
};

} // namespace

namespace El {

auto Args::instance() -> Args &
{
    static Args args{};
    return args;
}

void Args::printUsage(bool err, char const *appName)
{
    fmt::print(err ? stderr : stdout, USAGE, appName, DEFAULT_VAT * 100.0);
}

Args::Args()
    : _time{QDateTime::currentDateTime()}
{}

auto Args::init(int argc, char *argv[]) -> bool// NOLINT(modernize-avoid-c-arrays)
{
    using namespace Qt::Literals::StringLiterals;

    _region = u"ee"_s;

    char const *appName = argv[0];
    int         c       = 0;
    int         idx     = 0;
    while ((c = getopt_long(argc, argv, shortOpts, longOpts, &idx)) != -1) {
        switch (c) {

            case 'h': {
                printUsage(false, appName);
                return false;
            }

            case 'd': {
                char *e = nullptr;
                _day    = strtod(optarg, &e);
                if (e == nullptr || *e != '\0') {
                    fmt::print(stderr, "Vigane väärtus \"{}\" argumendile '--day'\n", optarg);
                    return false;
                }
                break;
            }

            case 'k': {
                if (optarg != nullptr) {
                    char *e = nullptr;
                    _km     = strtod(optarg, &e) / 100.0;
                    if (e == nullptr || (*e != '\0' && *e != '%')) {
                        fmt::print(stderr, "Vigane väärtus \"{}\" argumendile '--km'\n", optarg);
                        return false;
                    }
                    if (*e == '%' && *(e + 1) != '\0') {
                        fmt::print(stderr, "Vigane väärtus \"{}\" argumendile '--km'\n", optarg);
                        return false;
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
                    fmt::print(stderr, "Vigane väärtus \"{}\" argumendile '--margin'\n", optarg);
                    return false;
                }
                break;
            }

            case 'n': {
                char *e = nullptr;
                _night  = strtod(optarg, &e);
                if (e == nullptr || *e != '\0') {
                    fmt::print(stderr, "Vigane väärtus \"{}\" argumendile '--night'\n", optarg);
                    return false;
                }
                break;
            }

            case 't': {
                _time = QDateTime::fromString(optarg, u"yyyy-MM-dd hh:mm"_s);
                if (!_time.isValid()) {
                    fmt::print(stderr, "Vigane väärtus \"{}\" argumendile '--time'\n", optarg);
                    return false;
                }
                break;
            }

            case 'p': {
                _prices = true;
                if (optarg != nullptr) {
                    _priceFileName = optarg;
                }
                break;
            }

            case 'r': {
                _region = optarg;
                break;
            }

            case 'v': {
                _verbose = true;
                break;
            }

            case ':': {
                fmt::print(stderr, "Argumendi väärtus puudub\n\n");
                return false;
            }

            default: {
                printUsage(true, appName);
                return false;
            }
        }
    }

    // Verify that both day and night start values are given
    if (_day.has_value() != _night.has_value()) {
        fmt::print(stderr, "Nii päeva kui öö väärtused peavad olema antud\n");
        return false;
    }

    // Verify that the filename is given
    if (optind == argc) {
        fmt::print(stderr, "Faili nimi puudub\n\n");
        printUsage(true, appName);
        return false;
    }
    _fileName = argv[optind++];

    // Verify that only one filename is given
    if (optind != argc) {
        fmt::print(stderr, "Ainut üks failinimi võib olla antud\n\n");
        return false;
    }

    return true;
}

} // namespace El

#include "app.h"
#include "args.h"
#include "prices.h"

#include <QTimer>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>

#include <stdio.h>


// -----------------------------------------------------------------------------

App::App(Args const & args, int & argc, char ** argv)
    : QCoreApplication(argc, argv)
    , _args(args)
{
    QTimer::singleShot(0, this, &App::process);
}

App::~App() = default;


void App::process()
{
    // Load the CSV file
    if (!load_csv_file()) {
        exit(1);
        return;
    }

    // Load or request Nord Pool prices
    if (_args.prices()) {
        if (!_args.priceFileName().isEmpty()) {
            if (!load_prices_file()) {
                exit(1);
            }
        }
        else {
            if (!get_prices(_firstRecordTime, _lastRecordTime)) {
                exit(1);
            }

            // The network manager will continue the process
            return;
        }
    }

    QTimer::singleShot(0, this, &App::calc);
}

void App::calc()
{
    // Calculate and show totals
    if (!calc_summary()) {
        exit(1);
        return;
    }

    if (!show_summary()) {
        exit(1);
        return;
    }

    quit();
}

bool App::load_csv_file()
{
    // Open the input file
    QFile file(_args.fileName());
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        fprintf(stderr, "Failed to open the CSV file \"%s\": %s\n",
            qPrintable(_args.fileName()),
            qPrintable(file.errorString()));
        return false;
    }

    // Load all the records
    int lineno = 0;
    while (!file.atEnd()) {
        ++lineno;
        auto const line = file.readLine();
        if (lineno <= _args.skip()) {
            continue;
        }

        Record rec{lineno, line, _args.oldFormat()};
        if (!rec.isValid()) {
            continue;
        }

        // Verify time
        if (rec.endTime() > _args.time()) {
            break;
        }

        _records.push_back(rec);
    }

    if (_records.isEmpty()) {
        fprintf(stderr, "The CSV file contains no records\n");
        return false;
    }

    _firstRecordTime = _records.first().startTime();
    _lastRecordTime = _records.last().startTime();

    return true;
}

bool App::load_prices_file()
{
    _prices.reset(new Prices{_args});
    if (!_prices->loadFromFile(_args.priceFileName())) {
        _prices.reset();
        return false;
    }
    return true;
}

bool App::get_prices(QDateTime const & first, QDateTime const & last)
{
    QNetworkAccessManager * manager = new QNetworkAccessManager{this};
    connect(manager, &QNetworkAccessManager::finished, this, &App::get_prices_reply);

    QString q = QString{"https://dashboard.elering.ee/api/nps/price?start=%1&end=%2"}
                .arg(first.toUTC().toString("yyyy-MM-dThh\'\%3A\'mm\'\%3A\'ss.zzzZ"))
                .arg(last.toUTC().toString("yyyy-MM-ddThh\'\%3A\'mm\'\%3A\'ss.zzzZ"));

    QNetworkRequest rqst{};
    rqst.setUrl(QUrl{q});
    rqst.setRawHeader("accept", "*/*");
    manager->get(rqst);

    return true;
}

void App::get_prices_reply(QNetworkReply * reply)
{
    if (!reply) return;
    reply->deleteLater();

    auto const result = reply->readAll();
    if (result.isEmpty()) {
        fprintf(stderr, "Empty response from the Elering server\n");
        exit(1);
        return;
    }

    // Load prices
    _prices.reset(new Prices{_args});
    if (!_prices->loadFromJson(result)) {
        exit(1);
        return;
    }

    QTimer::singleShot(0, this, &App::calc);
}

bool App::calc_summary()
{
    // VAT multipler
    auto const vat = 1.0 + _args.km();

    for (auto const & rec : _records) {

        // Sum kWh
        if (rec.isNight()) {
            _night_kwh += rec.kWh();
        }
        else {
            _day_kwh += rec.kWh();
        }

        // Sum cost
        if (_prices && _prices->valid()) {
            auto const price = _prices->getPrice(rec.startTime());
            if (!price) {
                printf("WARNING: no price information for %s\n", qPrintable(rec.startTime().toString()));
                continue;
            }

            auto const cost = price.value() * rec.kWh();
            auto const margin = _args.margin() * rec.kWh();
            if (_args.verbose()) {
                printf("\t%24s\t%.3f kWh\t%.3f EUR\t@%.4f EUR\n", qPrintable(rec.startTime().toString()),
                                                                    rec.kWh(), cost * vat,
                                                                    price.value() * vat);
            }
            if (rec.isNight()) {
                _night_eur += (cost + margin);
            }
            else {
                _day_eur += (cost + margin);
            }
        }
    }

    return true;
}

bool App::show_summary()
{
    // VAT multipler
    auto const vat = 1.0 + _args.km();

    if (_args.startDay() && _args.startNight()) {
        printf("arvesti näit\n\töö: %10.3f\tpäev: %10.3f\n", _args.startNight().value() + _night_kwh,
                                                             _args.startDay().value() + _day_kwh);
    }
    printf("kulu kWh\n\töö: %10.3f kWh\tpäev: %10.3f kWh\tkokku: %10.3f kWh\n", _night_kwh, _day_kwh, _night_kwh + _day_kwh);
    if (_prices && _prices->valid()) {
        printf("kulu EUR\n\töö: %10.2f EUR\tpäev: %10.2f EUR\tkokku: %10.2f EUR\n",
                    _night_eur * vat, _day_eur * vat, (_night_eur + _day_eur) * vat);
        printf("hind EUR/kWh\n\töö: %6.4f EUR/kWh\tpäev: %6.4f EUR/kWh\tkeskmine: %6.4f EUR/kWh\n",
                    (_night_eur / _night_kwh) * vat,
                    (_day_eur / _day_kwh) * vat, ((_night_eur + _day_eur) / (_night_kwh + _day_kwh)) * vat);
    }
    return true;
}

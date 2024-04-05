#include "app.h"
#include "args.h"
#include "common.h"
#include "consumption.h"
#include "prices.h"

#include <QDateTime>
#include <QTimer>

#include <fmt/format.h>

#include <stdio.h>

namespace El {

// -----------------------------------------------------------------------------

App::App(Args const &args, int &argc, char **argv)
    : QCoreApplication(argc, argv)
    , _args(args)
    , _consumption(new Consumption{*this, this})
{
    QTimer::singleShot(0, this, &App::process);
}

App::~App() = default;

bool App::wait_for(bool const &flag, int ms)
{
    auto const start = QTime::currentTime().msecsSinceStartOfDay();

    // process events until `flag` becomes true or timeout
    while (!flag && ((QTime::currentTime().msecsSinceStartOfDay() - start) < ms)) {
        processEvents(QEventLoop::AllEvents | QEventLoop::WaitForMoreEvents, 100);
    }

    return flag;
}

void App::process()
{
    // Load the CSV file
    if (!_consumption->load(_args.fileName())) {
        exit(1);
        return;
    }

    // Load or request Nord Pool prices
    if (_args.prices()) {

        _prices = new Prices{*this, this};
        if (!_prices->load(_args.region(), _consumption->first_record_time(), _consumption->last_record_time())) {
            exit(1);
            return;
        }
    }

    // calculate and show results
    if (!calc()) {
        exit(1);
    }

    quit();
}

bool App::calc()
{
    // Calculate and show totals
    if (!calc_summary()) {
        return false;
    }

    if (!show_summary()) {
        return false;
    }

    return true;
}

bool App::calc_summary()
{
    // VAT multiplier
    auto const vat = 1.0 + _args.km();

    for (auto const &rec : _consumption->records()) {

        // Sum kWh
        if (rec.isNight()) {
            _night_kwh += rec.kWh();
        }
        else {
            _day_kwh += rec.kWh();
        }

        // Sum cost
        if (_prices) {
            auto const price = _prices->get_price(rec.startTime());
            if (!price) {
                fmt::print("WARNING: puudub hinnainfo ajale {}\n", rec.startTime());
                continue;
            }

            auto const cost   = price.value() * rec.kWh();
            auto const margin = _args.margin() * rec.kWh();
            if (_args.verbose()) {
                fmt::print("\t{}\t{:.3f} kWh\t{:.3f} EUR\t@{:.4f} EUR\n",
                       rec.startTime(),
                       rec.kWh(),
                       cost * vat,
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
        fmt::print("arvesti näit\n\töö: {:10.3f}\tpäev: {:10.3f}\n",
               _args.startNight().value() + _night_kwh,
               _args.startDay().value() + _day_kwh);
    }
    fmt::print("kulu kWh\n\töö: {:10.3f} kWh\tpäev: {:10.3f} kWh\tkokku: {:10.3f} kWh\n",
           _night_kwh,
           _day_kwh,
           _night_kwh + _day_kwh);
    if (_prices) {
        fmt::print("kulu EUR\n\töö: {:10.2f} EUR\tpäev: {:10.2f} EUR\tkokku: {:10.2f} EUR\n",
               _night_eur * vat,
               _day_eur * vat,
               (_night_eur + _day_eur) * vat);
        fmt::print("hind EUR/kWh\n\töö: {:6.4f} EUR/kWh\tpäev: {:6.4f} EUR/kWh\tkeskmine: {:6.4f} EUR/kWh\n",
               (_night_eur / _night_kwh) * vat,
               (_day_eur / _day_kwh) * vat,
               ((_night_eur + _day_eur) / (_night_kwh + _day_kwh)) * vat);
    }
    return true;
}

} // namespace El

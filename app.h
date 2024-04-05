#pragma once

#ifndef APP_H
#  define APP_H

#include <QCoreApplication>

namespace El {

class Args;
class Consumption;
class Prices;

class App : public QCoreApplication {
    Q_OBJECT

public:

    /// Wait for the `flag` to become `true`
    /// @param[in] flag The flag
    /// @param[in] ms Timeout in milliseconds
    /// @return The value of the `flag`
    static bool wait_for(bool const &flag, int ms);

    /// Ctor
    App(Args const &args, int &argc, char **argv);

    /// Dtor
    ~App() override;

    /// Arguments for the application
    inline auto const &args() const noexcept { return _args; }

private slots:

    void process();

private:

    /// Arguments for the application
    Args const &_args;

    /// Consumption records
    Consumption *_consumption = nullptr;

    /// Nord Pool prices
    Prices *_prices = nullptr;

    /// Total day consumption kWh
    double _day_kwh = 0.0;

    /// Total night consumption kWh
    double _night_kwh = 0.0;

    /// Total day cost EUR
    double _day_eur = 0.0;

    /// Total night cost EUR
    double _night_eur = 0.0;

    bool calc();
    bool calc_summary();
    bool show_summary();
};

} // namespace El

#endif

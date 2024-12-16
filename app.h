#pragma once

#ifndef APP_H
#  define APP_H

#include <QCoreApplication>

#include <memory>

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
    static auto wait_for(bool const &flag, int ms) -> bool;

    /// Ctor
    App(Args const &args, int &argc, char **argv);

    /// Dtor
    ~App() override;

    /// Arguments for the application
    inline auto const &args() const noexcept { return _args; }

private slots:

    void process();

private: // NOLINT

    /// Arguments for the application
    Args const &_args;

    /// Consumption records
    std::unique_ptr<Consumption> _consumption;

    /// Nord Pool prices
    std::unique_ptr<Prices> _prices;

    /// Total day consumption kWh
    double _day_kwh = 0.0;

    /// Total night consumption kWh
    double _night_kwh = 0.0;

    /// Total day cost EUR
    double _day_eur = 0.0;

    /// Total night cost EUR
    double _night_eur = 0.0;

    auto calc() -> bool;
    auto calc_summary() -> bool;
    auto show_summary() -> bool;
};

} // namespace El

#endif

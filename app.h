#pragma once

#ifndef APP_H
#define APP_H

#include "record.h"

#include <QCoreApplication>
#include <QDateTime>

#include <memory>

QT_BEGIN_NAMESPACE
    class QNetworkReply;
QT_END_NAMESPACE

class Args;
class Prices;

class App : public QCoreApplication {
    Q_OBJECT

public:

    /// Ctor
    App(Args const & args, int & argc, char ** argv);

    /// Dtor
    ~App() override;


private slots:

    void process();
    void calc();
    void get_prices_reply(QNetworkReply * reply);


private:

    bool load_csv_file();
    bool load_prices_file();
    bool get_prices(QDateTime const & from, QDateTime const & to);
    bool calc_summary();
    bool show_summary();


private:

    /// Arguments
    Args const & _args;

    /// Records from the CSV file
    QList<Record> _records;

    /// Date/time of the first record
    QDateTime _firstRecordTime;

    /// Date/time of the last record
    QDateTime _lastRecordTime;

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

};

#endif

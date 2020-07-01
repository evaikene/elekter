#include "record.h"

#include <QByteArray>
#include <QList>
#include <QLocale>

Record::Record(QByteArray const & line)
    : _valid(false)
    , _kWh(0.0)
    , _night(false)
{
    _valid = process(line);
}

bool Record::process(QByteArray const & line)
{
    bool ok = false;

    QList<QByteArray> fields = line.split(';');
    if (fields.size() < 3) {
        return ok;
    }

    // Start time
    _begin = QDateTime::fromString(fields.at(0), "dd.MM.yyyy hh:mm");

    // End time
    _end = QDateTime::fromString(fields.at(1), "dd.MM.yyyy hh:mm").addSecs(-60);

    // kWh
    QLocale locale(QLocale::Estonian, QLocale::Estonia);
    _kWh = locale.toDouble(fields.at(2), &ok);
    if (!ok) {
        return ok;
    }

    QDateTime nightStart(_begin.date(), QTime(23, 0));
    QDateTime nightEnd(_begin.date().addDays(1), QTime(7, 0));
    if (_begin.time() < QTime(7, 0)) {
        nightStart = nightStart.addDays(-1);
        nightEnd = nightEnd.addDays(-1);
    }
    if (_begin.isDaylightTime()) {
        nightStart = QDateTime(_begin.date(), QTime(0, 0));
        nightEnd = QDateTime(_begin.date(), QTime(8, 0));
        if (_begin.time() >= QTime(8, 0)) {
            nightStart = nightStart.addDays(1);
            nightEnd = nightEnd.addDays(1);
        }
    }
    if (_begin.date().dayOfWeek() == 6 || _begin.date().dayOfWeek() == 7 ||
            (_begin >= nightStart && _end < nightEnd)) {
        _night = true;
    }

    return ok;
}

#include "record.h"

#include <QByteArray>
#include <QList>
#include <QLocale>
#include <QStringLiteral>

#include <fmt/format.h>

namespace El {

Record::Record(int lineno, QByteArray const &line, bool old)
{
    _valid = process(lineno, line, old);
}

bool Record::process(int lineno, QByteArray const &line, bool old)
{
    QList<QByteArray> fields = line.split(';');
    if (fields.size() < (old ? 3 : 5)) {
        fmt::print("WARNING: Invalid number of fields on line #{}\n", lineno);
        return false;
    }

    // Start time
    size_t idx = 0;
    _begin     = QDateTime::fromString(fields.at(idx), "dd.MM.yyyy hh:mm");
    if (!_begin.isValid()) {
        fmt::print("WARNING: Invalid start time on line #{}\n", lineno);
        return false;
    }

    // End time
    ++idx;
    _end = QDateTime::fromString(fields.at(idx), "dd.MM.yyyy hh:mm").addSecs(-60);
    if (!_end.isValid()) {
        fmt::print("WARNING: Invalid end time on line #{}\n", lineno);
        return false;
    }

    // kWh
    if (old) {
        ++idx;
    }
    else {
        idx = 4;
    }
    bool    ok = false;
    QLocale locale(QLocale::Estonian, QLocale::Estonia);
    _kWh = locale.toDouble(fields.at(idx), &ok);
    if (!ok) {
        // if failed, try without the locale
        _kWh = fields.at(2).toDouble(&ok);
    }
    if (!ok) {
        fmt::print("WARNING: Invalid consumption value on line #{}\n", lineno);
        return false;
    }

    if (old) {
        QDateTime nightStart(_begin.date(), QTime(23, 0));
        QDateTime nightEnd(_begin.date().addDays(1), QTime(7, 0));
        if (_begin.time() < QTime(7, 0)) {
            nightStart = nightStart.addDays(-1);
            nightEnd   = nightEnd.addDays(-1);
        }
        if (_begin.isDaylightTime()) {
            nightStart = QDateTime(_begin.date(), QTime(0, 0));
            nightEnd   = QDateTime(_begin.date(), QTime(8, 0));
            if (_begin.time() >= QTime(8, 0)) {
                nightStart = nightStart.addDays(1);
                nightEnd   = nightEnd.addDays(1);
            }
        }
        if (_begin.date().dayOfWeek() == 6 || _begin.date().dayOfWeek() == 7 ||
            (_begin >= nightStart && _end < nightEnd)) {
            _night = true;
        }
    }
    else {
        idx    = 2;
        _night = (QString::fromUtf8(fields.at(idx)).compare(QStringLiteral(u"öö"), Qt::CaseInsensitive) == 0);
    }

    return true;
}

} // namespace El

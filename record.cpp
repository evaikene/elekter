#include "record.h"

#include <QByteArray>
#include <QList>
#include <QLocale>

#include <fmt/format.h>

namespace El {

Record::Record(int lineno, QByteArray const &line, bool old)
{
    _valid = process(lineno, line, old);
}

auto Record::process(int lineno, QByteArray const &line, bool old) -> bool
{
    using namespace Qt::Literals::StringLiterals;

    constexpr int OLD_FIELDS_SZ = 3;
    constexpr int NEW_FIELDS_SZ = 5;
    constexpr int SECS_IN_MIN   = 60;

    QList<QByteArray> fields = line.split(';');
    if (fields.size() < (old ? OLD_FIELDS_SZ : NEW_FIELDS_SZ)) {
        fmt::print("WARNING: Invalid number of fields on line #{}\n", lineno);
        return false;
    }

    // Start time
    int idx = 0;
    _begin  = QDateTime::fromString(fields.at(idx), u"dd.MM.yyyy hh:mm"_s);
    if (!_begin.isValid()) {
        fmt::print("WARNING: Invalid start time on line #{}\n", lineno);
        return false;
    }

    // End time
    ++idx;
    _end = QDateTime::fromString(fields.at(idx), u"dd.MM.yyyy hh:mm"_s).addSecs(-SECS_IN_MIN);
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
        constexpr int NIGHT_START = 23;
        constexpr int NIGHT_END   = 7;
        QDateTime nightStart(_begin.date(), QTime(NIGHT_START, 0));
        QDateTime nightEnd(_begin.date().addDays(1), QTime(NIGHT_END, 0));
        if (_begin.time() < QTime(NIGHT_END, 0)) {
            nightStart = nightStart.addDays(-1);
            nightEnd   = nightEnd.addDays(-1);
        }
        if (_begin.isDaylightTime()) {
            constexpr int NIGHT_START_DST = 24;
            constexpr int NIGHT_END_DST = 8;
            nightStart = QDateTime(_begin.date(), QTime(NIGHT_START_DST, 0));
            nightEnd   = QDateTime(_begin.date(), QTime(NIGHT_END_DST, 0));
            if (_begin.time() >= QTime(NIGHT_END_DST, 0)) {
                nightStart = nightStart.addDays(1);
                nightEnd   = nightEnd.addDays(1);
            }
        }

        constexpr int DOW_SAT = 6;
        constexpr int DOW_SUN = 7;
        if (_begin.date().dayOfWeek() == DOW_SAT || _begin.date().dayOfWeek() == DOW_SUN ||
            (_begin >= nightStart && _end < nightEnd)) {
            _night = true;
        }
    }
    else {
        idx    = 2;
        _night = (QString::fromUtf8(fields.at(idx)).compare(u"öö"_s, Qt::CaseInsensitive) == 0);
    }

    return true;
}

} // namespace El

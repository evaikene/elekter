#include "record.h"
#include "header.h"

#include <QByteArray>
#include <QList>
#include <QLocale>

#include <fmt/format.h>

namespace El {

Record::Record(int lineno, QByteArray const &line, Header const &hdr)
{
    _valid = process(lineno, line, hdr);
}

auto Record::process(int lineno, QByteArray const &line, Header const &hdr) -> bool
{
    using namespace Qt::Literals::StringLiterals;

    constexpr int SECS_IN_MIN   = 60;
    constexpr int SECS_IN_HOUR  = 3'600;

    QList<QByteArray> fields = line.split(';');
    if (fields.size() < hdr.numFields()) {
        fmt::print("WARNING: Invalid number of fields on line #{}\n", lineno);
        return false;
    }

    // Start time
    _begin  = QDateTime::fromString(fields.at(hdr.idxStartTime()), u"dd.MM.yyyy hh:mm"_s);
    if (!_begin.isValid()) {
        fmt::print("WARNING: Invalid start time on line #{}\n", lineno);
        return false;
    }

    // End time
    if (hdr.idxEndTime() >= 0) {
        _end = QDateTime::fromString(fields.at(hdr.idxEndTime()), u"dd.MM.yyyy hh:mm"_s).addSecs(-SECS_IN_MIN);
        if (!_end.isValid()) {
            fmt::print("WARNING: Invalid end time on line #{}\n", lineno);
            return false;
        }
    }
    else {
        _end = _begin.addSecs(SECS_IN_HOUR - SECS_IN_MIN);
    }

    // kWh
    bool    ok = false;
    QLocale locale(QLocale::Estonian, QLocale::Estonia);
    _kWh = locale.toDouble(fields.at(hdr.idxConsumption()), &ok);
    if (!ok) {
        // if failed, try without the locale
        _kWh = fields.at(2).toDouble(&ok);
    }
    if (!ok) {
        fmt::print("WARNING: Invalid consumption value on line #{}\n", lineno);
        return false;
    }

    if (hdr.idxConsumptionType() < 0) {
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
        _night = QString::fromUtf8(fields.at(hdr.idxConsumptionType())).contains(u"öö"_s, Qt::CaseInsensitive);
    }

    return true;
}

} // namespace El

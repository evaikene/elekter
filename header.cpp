#include "header.h"
#include "common.h" // IWYU pragma: keep Needed for formatting Qt types

#include <QByteArray>
#include <QList>
#include <QString>

#include <fmt/format.h>

namespace El {

// -----------------------------------------------------------------------------

Header::Header(QByteArray const &line)
{
    _valid = process(line);

    if (!_valid) {
        fmt::print(stderr, "ERROR: Invalid header '{}'\n", line);
    }
}

auto Header::process(QByteArray const &line) -> bool
{
    using namespace Qt::Literals::StringLiterals;

    // split the header line into fields
    auto const fields = line.split(';');

    // the number of expected fields must be equal to the number of header fields
    _num_fields = fields.size();

    // try to recognize the fields
    qsizetype idx = 0;
    for (auto const &h : fields) {
        auto const header = QString::fromUtf8(h).trimmed();
        if (_idx_start_time < 0 && header.contains(u"algus"_s, Qt::CaseInsensitive)) {
            _idx_start_time = idx;
        }
        else if (_idx_end_time < 0 && header.contains(u"lõpp"_s, Qt::CaseInsensitive)) {
            _idx_end_time = idx;
        }
        else if (_idx_consumption < 0 &&
            (header.contains(u"tarbimine"_s, Qt::CaseInsensitive) ||
             header.contains(u"kogus"_s, Qt::CaseInsensitive) ||
             header.contains(u"aktiiv"_s))) {
            _idx_consumption = idx;
        }
        else if (_idx_consumption_type < 0 &&
                 header.contains(u"päev"_s, Qt::CaseInsensitive) &&
                 header.contains(u"öö"_s)) {
            _idx_consumption_type = idx;
        }

        ++idx;
    }

    return _num_fields > 0 && _idx_start_time >= 0 && _idx_consumption >= 0;
}

} // namespace El

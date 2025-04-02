#include "consumption.h"
#include "app.h"
#include "args.h"
#include "common.h" // IWYU pragma: keep Needed for formatting Qt types
#include "header.h"

#include <QFile>

#include <fmt/format.h>

namespace El {

Consumption::Consumption(App const &app)
    : _app(app)
{}

Consumption::~Consumption() = default;

auto Consumption::load(QString const &filename) -> bool
{
    // open the input file
    QFile file(filename);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        fmt::print(stderr, "CSV faili {} avamine ebaõnnestus: {}", filename, file.errorString());
        return false;
    }

    // load all the records
    int lineno = 0;
    bool skip = true;
    bool header = true;
    Header hdr;
    while (!file.atEnd()) {
        ++lineno;

        auto const line = file.readLine().trimmed();

        // skip the first lines until we reach the header
        if (skip) {
            // there is an empty line or a line with just two quotes between the beginning and header
            skip = !line.isEmpty() && line != "\"\"";
            continue;
        }

        // read the header
        if (header) {
            header = false;

            hdr = Header(line);
            if (!hdr.isValid()) {
                return false;
            }
            continue;
        }

        Record rec{lineno, line, hdr};
        if (!rec.isValid()) {
            continue;
        }

        // verify time
        if (rec.endTime() > _app.args().time()) {
            break;
        }

        _records.append(std::move(rec));
    }

    // ensure that there is at least one record
    if (_records.isEmpty()) {
        fmt::print(stderr, "CSV fail ei sisalda ühtegi kirjet\n");
        return false;
    }

    _first_record_time = _records.first().startTime();
    _last_record_time = _records.last().startTime();

    return true;
}

} // namespace El

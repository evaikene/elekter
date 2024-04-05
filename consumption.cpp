#include "consumption.h"
#include "app.h"
#include "args.h"
#include "common.h" // needed for fmt

#include <QFile>

#include <fmt/format.h>

namespace El {

Consumption::Consumption(App const &app, QObject *parent)
    : QObject(parent)
    , _app(app)
{}

bool Consumption::load(QString const &filename)
{
    // open the input file
    QFile file(filename);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        fmt::print(stderr, "CSV faili {} avamine ebaõnnestus: {}", filename, file.errorString());
        return false;
    }

    // load all the records
    int lineno = 0;
    while (!file.atEnd()) {
        ++lineno;

        auto const line = file.readLine();
        if (lineno <= _app.args().skip()) continue;

        Record rec{lineno, line, _app.args().oldFormat()};
        if (!rec.isValid()) continue;

        // verify time
        if (rec.endTime() > _app.args().time()) break;

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

}

#include "args.h"
#include "record.h"

#include <QFile>
#include <QList>

#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"

int main(int argc, char * argv[])
{
    Args args(argc, argv);
    if (!args.isValid()) {
        return EXIT_FAILURE;
    }

    // Open the input file
    QFile file(args.fileName());
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        fprintf(stderr, "Failed to open file \"%s\": %s\n",
                args.fileName().constData(),
                qPrintable(file.errorString()));
    }

    // Process records
    QList<Record> records;
    double day = args.startDay();
    double night = args.startNight();
    int lineno = 0;
    while (!file.atEnd()) {
        ++lineno;
        QByteArray const line = file.readLine();
        if (lineno <= args.skip()) {
            continue;
        }
        if (line.isEmpty()) {
            continue;
        }

        Record rec(lineno, line);
        if (!rec.isValid()) {
            continue;
        }

        // Verify time
        if (rec.endTime() > args.time()) {
            break;
        }

        if (rec.isNight()) {
            night += rec.kWh();
        }
        else {
            day += rec.kWh();
        }

        records << rec;
    }

    printf("night: %.3f\tday: %.3f\ttotal: %.3f\n", night, day, night + day);

    return EXIT_SUCCESS;
}

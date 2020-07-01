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

    // Skip n lines from the beginning
    for (int i = 0; i < args.skip(); ++i) {
        file.readLine();
    }

    // Process records
    QList<Record> records;
    double day = args.startDay();
    double night = args.startNight();
    while (!file.atEnd()) {
        QByteArray const line = file.readLine();
        if (line.isEmpty()) {
            continue;
        }

        Record rec(line);
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

    printf("night: %.3f\tday: %.3f\n", night, day);

    return EXIT_SUCCESS;
}

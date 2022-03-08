#include "args.h"
#include "prices.h"
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
        return EXIT_FAILURE;
    }

    Prices prices(args);
    if (!args.priceFileName().isEmpty()) {
        if (!prices.loadFromFile(args.priceFileName())) {
            return EXIT_FAILURE;
        }
    }

    // Process records
    double day = 0.0;
    double night = 0.0;
    double day_cost = 0.0;
    double night_cost = 0.0;
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

        Record rec(lineno, line, args.oldFormat());
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

        // Calculate cost
        if (prices.valid()) {
            auto const price = prices.getPrice(rec.startTime());
            if (!price) {
                printf("WARNING: no price information for %s\n", qPrintable(rec.startTime().toString()));
            }
            else {
                auto const cost = price.value() * rec.kWh();
                auto const margin = args.margin() * rec.kWh();
                if (args.verbose()) {
                    printf("\t%24s\t%.3f kWh\t%.3f EUR\t@%.4f EUR\n", qPrintable(rec.startTime().toString()), rec.kWh(), cost * 1.2, price.value() * 1.2);
                }
                if (rec.isNight()) {
                    night_cost += (cost + margin);
                }
                else {
                    day_cost += (cost + margin);
                }
            }
        }
    }

    if (args.startDay() && args.startNight()) {
        printf("arvesti näit\n\töö: %10.3f\tpäev: %10.3f\n", night + args.startNight().value(), day + args.startDay().value());
    }
    printf("kulu kWh\n\töö: %10.3f kWh\tpäev: %10.3f kWh\tkokku: %10.3f kWh\n", night, day, night + day);
    if (prices.valid()) {
        printf("kulu EUR\n\töö: %10.2f EUR\tpäev: %10.2f EUR\tkokku: %10.2f EUR\n",
                    night_cost, day_cost, night_cost + day_cost);
        printf("hind EUR/kWh\n\töö: %6.4f EUR/kWh\tpäev: %6.4f EUR/kWh\tkeskmine: %6.4f EUR/kWh\n",
                    night_cost / night, day_cost / day, (night_cost + day_cost) / (night + day));
    }

    return EXIT_SUCCESS;
}

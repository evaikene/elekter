#include "args.h"
#include "app.h"

#include "unistd.h"

int main(int argc, char * argv[])
{
    El::Args args(argc, argv);
    if (!args.isValid()) {
        return EXIT_FAILURE;
    }

    // Create and run the application
    El::App app(args, argc, argv);
    return app.exec();
}

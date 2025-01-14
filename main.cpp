#include "args.h"
#include "app.h"

#include <cstdlib>

auto main(int argc, char *argv[]) -> int
{
    El::Args args{argc, argv};
    if (!args.isValid()) {
        return EXIT_FAILURE;
    }

    // Create and run the application
    El::App app{args, argc, argv};
    return El::App::exec();
}

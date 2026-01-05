#include "args.h"
#include "app.h"

#include <cstdlib>

auto main(int argc, char *argv[]) -> int
{
    if (!El::Args::instance().init(argc, argv)) {
        return EXIT_FAILURE;
    }

    // Create and run the application
    El::App app{argc, argv};
    return El::App::exec();
}

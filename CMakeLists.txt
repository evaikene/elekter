cmake_minimum_required (VERSION 3.12...3.27 FATAL_ERROR)

project (elekter CXX)

set (CMAKE_CXX_STANDARD 17)

find_package (Qt5 COMPONENTS Core Network REQUIRED)
find_package (fmt REQUIRED)

set(CMAKE_AUTOMOC ON)

set (HDRS
    app.h
    args.h
    common.h
    json.h
    nordpool.h
    prices.h
    record.h
)
set (SRCS
    app.cpp
    args.cpp
    common.cpp
    json.cpp
    main.cpp
    nordpool.cpp
    prices.cpp
    record.cpp
)
add_executable (${PROJECT_NAME} ${HDRS} ${SRCS})
target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Network fmt::fmt)

cmake_minimum_required (VERSION 3.12...3.27 FATAL_ERROR)

project (elekter CXX)

set (CMAKE_CXX_STANDARD 17)

find_package (Qt6 REQUIRED COMPONENTS Core Network Sql)
find_package (fmt REQUIRED)

set(CMAKE_AUTOMOC ON)

set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

set (HDRS
    app.h
    args.h
    cache.h
    common.h
    consumption.h
    json.h
    nordpool.h
    prices.h
    record.h
)
set (SRCS
    app.cpp
    args.cpp
    cache.cpp
    common.cpp
    consumption.cpp
    json.cpp
    main.cpp
    nordpool.cpp
    prices.cpp
    record.cpp
)
add_executable (${PROJECT_NAME} ${HDRS} ${SRCS})
target_link_libraries(${PROJECT_NAME} Qt6::Core Qt6::Network Qt6::Sql fmt::fmt)
install(TARGETS ${PROJECT_NAME})

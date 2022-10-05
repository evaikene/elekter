project (elekter CXX)
cmake_minimum_required (VERSION 3.7)
set (CMAKE_CXX_STANDARD 17)

find_package (Qt5 COMPONENTS Core Network REQUIRED)

set(CMAKE_AUTOMOC ON)

set (HDRS
    app.h
    args.h
    prices.h
    record.h
)
set (SRCS
    app.cpp
    args.cpp
    main.cpp
    prices.cpp
    record.cpp
)
add_executable (${PROJECT_NAME} ${HDRS} ${SRCS})
target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Network)

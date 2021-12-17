project (elekter CXX)
cmake_minimum_required (VERSION 3.7)
set (CMAKE_CXX_STANDARD 17)

find_package (Qt5 COMPONENTS Core REQUIRED)

set (HDRS
    args.h
    prices.h
    record.h
)
set (SRCS
    args.cpp
    main.cpp
    prices.cpp
    record.cpp
)
add_executable (${PROJECT_NAME} ${HDRS} ${SRCS})
target_link_libraries(${PROJECT_NAME} Qt5::Core)

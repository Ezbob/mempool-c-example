CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

PROJECT(mempool)

SET(SOURCES 
    mempool.c
    main.c
)

SET(CMAKE_C_FLAGS "-Wall -Wextra -std=c99")

ADD_EXECUTABLE(mempool "")
TARGET_SOURCES(mempool PRIVATE ${SOURCES})
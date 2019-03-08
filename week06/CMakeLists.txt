cmake_minimum_required(VERSION 3.12)
project(week06)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(app
        src/app.c
        src/client.c
        src/event_loop.c
        src/known_nodes_hashmap.c
        src/main.c
        src/server.c)

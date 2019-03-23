cmake_minimum_required(VERSION 3.12)
project(week06)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_FLAGS -Iinclude)

include_directories(.)

add_executable(app
        src/event_loop.c
        src/known_nodes_hashmap.c
        src/main.c
        src/server.c
        src/logging.c
        src/requests.c
        src/client_context.c
        src/bootstrap.c src/handlers.c include/pings.h src/pings.c)

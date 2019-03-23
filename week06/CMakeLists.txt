cmake_minimum_required(VERSION 3.12)
project(week06)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_FLAGS -Iinclude)

include_directories(.)

add_executable(app
        src/event_loop.c
        src/known_nodes.c
        src/main.c
        src/server.c
        src/logging.c
        src/requests.c
        src/bootstrap.c
        src/handlers.c
        src/pings.c
        src/control.c
        src/ds/hashmap.c
        src/known_files.c
        src/my_files.c
        src/ds/json/interface.c
        src/ds/json/parser.c include/serializers.h src/serializers.c include/file_transfer.h src/file_transfer.c
        src/ds/vector.c
        src/ds/refcounter.c)

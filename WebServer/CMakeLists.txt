set(SRCS
        http_conn.cpp
        main.cpp
        )
include_directories(./base)

add_executable(WebServer ${SRCS})
target_link_libraries(WebServer server_base)

add_subdirectory(base)
#add_subdirectory(tests)
set(LIB_SRC
        Thread.cpp
        CountDownLatch.cpp
        )

add_library(server_base ${LIB_SRC})
target_link_libraries(server_base pthread)

add_subdirectory(tests)
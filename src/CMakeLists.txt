add_executable(main main.cpp)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_link_libraries(main PRIVATE Threads::Threads alpaca::alpaca)

project( alterstack_load_tests )

add_executable( load_futex ./load_futex.cpp )
target_link_libraries( load_futex Threads::Threads )

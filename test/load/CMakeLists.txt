project( alterstack_load_tests )

add_executable( load_futex ./load_futex.cpp )
target_link_libraries( load_futex Threads::Threads )

add_executable( load_bound_buffer
    load_bound_buffer.cpp
)
target_link_libraries( load_bound_buffer ${COMMON_LIBS} Threads::Threads )

add_executable( load_lock_free_queue
    load_lock_free_queue.cpp
)
target_link_libraries( load_lock_free_queue ${COMMON_LIBS} Threads::Threads)

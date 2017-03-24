project( alterstack_tests )

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

if( ALTERSTACK_USE_JEMALLOC )
    set( COMMON_LIBS ${COMMON_LIBS} -ljemalloc )
endif()

add_subdirectory( integration )

set(COMMON_LIBS ${COMMON_LIBS} -latomic)

#add_executable(test_gai
#    TestGai.cpp
#)
#target_link_libraries(test_gai alterstack ${Boost_LIBRARIES} ${COMMON_LIBS} -pthread -lanl)

#add_test( TestGai test_gai )

#add_executable(test_main
#    RunTasks.cpp
#)
#target_link_libraries(test_main alterstack ${Boost_LIBRARIES} ${COMMON_LIBS} -pthread)

#add_test( MainTest test_main )

#add_executable(test_factorial
#    CatchMain.cpp
#    Factorial.cpp
#)
#target_link_libraries(test_factorial ${COMMON_LIBS} )

#add_test( FactorialTest test_factorial )

#add_subdirectory(environment/moveable_params)

add_executable( gai_check
    gai_check.cpp
)
target_link_libraries( gai_check ${COMMON_LIBS} )

add_library( catch_main STATIC catch_main.cpp )

add_executable( task_buffer
    task_buffer.cpp
)
target_link_libraries( task_buffer catch_main ${COMMON_LIBS})
add_test( task_buffer task_buffer )

# load test
add_executable( load_task_buffer
    ./load_test/load_task_buffer.cpp
)
target_link_libraries( load_task_buffer ${COMMON_LIBS} Threads::Threads )

add_executable( task_stack
    task_stack.cpp
)
target_link_libraries( task_stack catch_main ${COMMON_LIBS} )
add_test( task_stack task_stack )

add_executable( running_queue
    running_queue.cpp
)
target_link_libraries( running_queue catch_main ${COMMON_LIBS} )
add_test( running_queue running_queue )
# load test
add_executable( load_running_queue
    ./load_test/load_running_queue.cpp
)
target_link_libraries( load_running_queue ${COMMON_LIBS} Threads::Threads)

add_executable( load_futex ./load_test/load_futex.cpp )
target_link_libraries( load_futex Threads::Threads )

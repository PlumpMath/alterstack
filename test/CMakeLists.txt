project( alterstack_tests )

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
set(COMMON_LIBS ${COMMON_LIBS} -ljemalloc -latomic)

add_executable( test_task_run_simple
    TaskRunSimple.cpp
)
target_link_libraries( test_task_run_simple alterstack ${COMMON_LIBS} Threads::Threads)

add_test( TaskRunSimpleTest test_task_run_simple )

add_executable( test_task_run
    TaskRun.cpp
)
target_link_libraries( test_task_run alterstack ${COMMON_LIBS} Threads::Threads )

add_test( TaskRunTest test_task_run )

add_executable( test_task_wait
    TaskWait.cpp
)
target_link_libraries( test_task_wait alterstack ${COMMON_LIBS} Threads::Threads )

add_test( TaskWaitTest test_task_wait )

add_executable( test_task_yield
    TaskYield.cpp
)
target_link_libraries( test_task_yield alterstack ${COMMON_LIBS} Threads::Threads )

add_test( TaskYieldTest test_task_yield )

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

add_executable( test_gai_test
    Gai_test.cpp
)
target_link_libraries( test_gai_test ${COMMON_LIBS} )

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

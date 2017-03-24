project( alterstack_integration_tests )

add_executable( task_run_simple
    task_run_simple.cpp
)
target_link_libraries( task_run_simple alterstack ${COMMON_LIBS} Threads::Threads)
add_test( task_run_simple task_run_simple )

add_executable( task_run
    task_run.cpp
)
target_link_libraries( task_run alterstack ${COMMON_LIBS} Threads::Threads )
add_test( task_run task_run )

add_executable( task_wait
    task_wait.cpp
)
target_link_libraries( task_wait alterstack ${COMMON_LIBS} Threads::Threads )
add_test( task_wait task_wait )

add_executable( task_yield
    task_yield.cpp
)
target_link_libraries( task_yield alterstack ${COMMON_LIBS} Threads::Threads )
add_test( task_yield task_yield )

project( alterstack_tests )

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

if( ALTERSTACK_USE_JEMALLOC )
    set( COMMON_LIBS ${COMMON_LIBS} -ljemalloc )
endif()
set(COMMON_LIBS ${COMMON_LIBS} -latomic)

add_subdirectory( integration )
add_subdirectory( load )
add_subdirectory( unit )

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

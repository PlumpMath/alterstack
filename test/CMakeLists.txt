project( alterstack_tests )

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

if( ALTERSTACK_USE_JEMALLOC )
    set( COMMON_LIBS ${COMMON_LIBS} -ljemalloc )
endif()
set(COMMON_LIBS ${COMMON_LIBS} -latomic)

add_subdirectory( integration )
add_subdirectory( load )
add_subdirectory( unit )

add_executable( gai_check
    gai_check.cpp
)
target_link_libraries( gai_check ${COMMON_LIBS} )

add_library( catch_main STATIC catch_main.cpp )

project( alterstack_unit_ests )

add_executable( unit_bound_buffer
    bound_buffer.cpp
)
target_link_libraries( unit_bound_buffer catch_main ${COMMON_LIBS})
add_test( unit_bound_buffer unit_bound_buffer )

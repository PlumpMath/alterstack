project( alterstack_unit_ests )

add_executable( unit_bound_buffer
    bound_buffer.cpp
)
target_link_libraries( unit_bound_buffer catch_main ${COMMON_LIBS})
add_test( unit_bound_buffer unit_bound_buffer )

add_executable( unit_lock_free_stack
    unit_lock_free_stack.cpp
)
target_link_libraries( unit_lock_free_stack catch_main ${COMMON_LIBS} )
add_test( unit_lock_free_stack unit_lock_free_stack )


project(shcdriver)

add_library(bfrtdriver STATIC
    base_table.cpp
    # hal_common_api.cpp
    hal_init.cpp
    hal_table_api.cpp

    tunnel_map_vrf.cpp
    route.cpp
    route_sele.cpp
    nhop.cpp
    neigh.cpp
    port_for_mac.cpp

    shc_digest.cpp
    shc_aging.cpp

    top_k/space_saving.cpp
    top_k/BOBHash32.cpp
    
    # utils/log.cpp
    # utils/object_id.cpp
)

# add_dependencies(bfrtdriver zdw_cntl_lib)

# target_link_libraries(bfrtdriver zdw_cntl_lib)
target_include_directories(bfrtdriver #PUBLIC ${LIBFMT_INCLUDE_DIR}
                                      PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                                      PRIVATE $ENV{SDE_INSTALL}/include)

set(SHC_DRIVER_INCLUDE_DIR  ${CMAKE_CURRENT_SOURCE_DIR} CACHE STRING INTERNAL)
message("public_private_dirs: ${SHC_DRIVER_INCLUDE_DIR}")
message("public_private_dirs: ${public_private_dirs}")
add_subdirectory(test_hal)
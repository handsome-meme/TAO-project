link_directories($ENV{SDE_INSTALL}/lib/)

add_executable(driver_test
    main.cpp
    tunnel_map_vrf_test.cpp
    route_test.cpp
    route_sele_test.cpp
    nhop_test.cpp
    neigh_test.cpp
    port_for_mac_test.cpp
)

add_dependencies(driver_test
    bfrtdriver
    # LIBFMT
)

target_link_libraries(driver_test
    bfutils
    bfsys
    driver
    bfrtdriver
    dl
    pthread
	ssl
	crypto
	thrift
	protobuf
	# grpc
	# grpc++
	# grpc_unsecure
	# grpc++_unsecure
	# grpc_cronet
	# gpr
	event
	# grpc++_reflection
	# jsoncpp
	curl
    clish
)

target_include_directories(
    driver_test PRIVATE ${SHC_DRIVER_INCLUDE_DIR}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE $ENV{SDE_INSTALL}/include
    # PRIVATE ${LIBFMT_INCLUDE_DIR}
    # PRIVATE ${SHC_ALARM_INCLUDE_DIR}
)

# Binaries
install (TARGETS driver_test DESTINATION /share/proj_execute)
        #  COMPONENT cntl
        #  PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ
        #  WORLD_EXECUTE WORLD_READ)
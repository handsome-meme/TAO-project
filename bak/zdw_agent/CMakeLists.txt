link_directories($ENV{SDE_INSTALL}/lib/)

add_library(zdw_cntl_lib STATIC vrf.c
    tunnel_map.c
    route.c
    pmd.c
    netio.c
    neigh.c
)

set(LIBDPDK_CFLAGS 
       -include rte_config.h 
       -march=native
       -DALLOW_EXPERIMENTAL_API
)

set(LIBDPDK_LIBS 
    rte_fib
	rte_rib
	rte_hash
	rte_mbuf
	rte_mempool
	rte_rcu
	rte_ring
	rte_eal
	rte_telemetry
	rte_kvargs
	numa
)

set(LIBSTAND_LIBS
    bfutils
	bfsys
	driver
	ssl
	crypto
	pthread
	thrift
	protobuf
	grpc
	grpc++
	grpc_unsecure
	grpc++_unsecure
	grpc_cronet
	gpr
	dl
	event
	grpc++_reflection
	jsoncpp
	curl
)

target_compile_options(zdw_cntl_lib PRIVATE ${LIBDPDK_CFLAGS})

target_link_libraries(zdw_cntl_lib PUBLIC
    ${LIBDPDK_LIBS}
    driver
    zmq
    ${LIBSTAND_LIBS}
    ${CJSON_LIB}
    ${LIBFMT_LIB}
)

target_include_directories(
    zdw_cntl_lib PUBLIC ${NTB_ALARM_INCLUDE_DIR}
    ${NTB_COLLECT_INCLUDE_DIR}
    ${NTB_DRIVER_INCLUDE_DIR}
    ${CJSON_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    $ENV{SDE_INSTALL}/include
    /usr/include/dpdk
    ${LIBFMT_INCLUDE_DIR}
)

#======================== zdw_ctl ========================
add_executable(zdw_cntl main.c)

add_dependencies(zdw_cntl
    zdw_cntl_lib
)

target_link_libraries(zdw_cntl
    zdw_cntl_lib
)

# Binaries
install (TARGETS zdw_cntl DESTINATION /opt/file/install/bin/
         COMPONENT cntl
         PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ
         WORLD_EXECUTE WORLD_READ)

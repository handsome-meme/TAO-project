link_directories($ENV{SDE_INSTALL}/lib/)

add_library(ntb_cntl_lib STATIC vrf.c
    pmd.c
    route.c
    neigh.c
    stat.c
    tunnel_map.c
    ntb_signal.c
    ntb_tunnel_stat.cpp
    ntb_utils.cpp
    ntb_log.cpp
    ntb_ov_cfg.cpp
    ntb_pltfm.cpp
    ntb_port_stat.cpp
    netio.c
    ntb_api.c
    ntb_cfg.c
    ntb_dp_err_monitor.cpp
    ntb_hal_tb_usage_mon.cpp
    ntb_errno.c
    flow.c
    device.c
    acl.c
    ntb_spec.c
    ntb_tm.cpp
    ntb_grpc/ntb_grpc_comm.cpp
    ntb_grpc/ntb_grpc_config.cpp
    ntb_grpc/ntb_grpc_report.cpp
    ntb_grpc/ntb_grpc_server.cpp
    ntb_grpc/ntb_grpc_cli.cpp
    ntb_grpc/ntb_grpc_confcheck.cpp
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
	numa)

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

target_compile_options(ntb_cntl_lib PRIVATE ${LIBDPDK_CFLAGS})

add_dependencies(ntb_cntl_lib
    ntb_proto
    ntb_alarm
    ntb_collect
    ntb_zmq
    LIBFMT
    CJSON
    bfrtdriver
    P4
)

target_link_libraries(ntb_cntl_lib PUBLIC
    ${LIBDPDK_LIBS}
    ntb_proto
    ntb_alarm
    ntb_collect
    ntb_zmq
    bfrtdriver
    driver
    zmq
    ${LIBSTAND_LIBS}
    ${CJSON_LIB}
    ${LIBFMT_LIB}
)

target_include_directories(
    ntb_cntl_lib PUBLIC ${NTB_ALARM_INCLUDE_DIR}
    ${NTB_COLLECT_INCLUDE_DIR}
    ntb_grpc/
    ${NTB_DRIVER_INCLUDE_DIR}
    ${CJSON_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    $ENV{SDE_INSTALL}/include
    /usr/include/dpdk
    ${LIBFMT_INCLUDE_DIR}
)

#======================== ntb_ctl ========================
add_executable(ntb_cntl main.c)

add_dependencies(ntb_cntl
    ntb_cntl_lib
)

target_link_libraries(ntb_cntl
    ntb_cntl_lib
)

# Binaries
install (TARGETS ntb_cntl DESTINATION /opt/file/install/bin/
         COMPONENT cntl
         PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ
         WORLD_EXECUTE WORLD_READ)

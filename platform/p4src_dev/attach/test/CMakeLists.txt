link_directories($ENV{SDE_INSTALL}/lib/)

add_executable(inset_test
    main.cpp
    route.cpp
)

# add_dependencies(inset_test
#     # p4
#     # LIBFMT
# )

target_link_libraries(inset_test
    bfutils
    bfsys
    driver
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
    inset_test PRIVATE ${SHC_DRIVER_INCLUDE_DIR}
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE $ENV{SDE_INSTALL}/include
    # PRIVATE ${LIBFMT_INCLUDE_DIR}
    # PRIVATE ${SHC_ALARM_INCLUDE_DIR}
)

# Binaries
install (TARGETS inset_test DESTINATION /share/proj_execute)
        #  COMPONENT cntl
        #  PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ
        #  WORLD_EXECUTE WORLD_READ)
include(ExternalProject)

set(P4_ROOT ${CMAKE_BINARY_DIR}/thirdparty/p4)
set(P4_SOURCE_DIR   ${PROJECT_SOURCE_DIR}/platform/p4src)
set(P4_CONFIGURE    cd ${P4_ROOT}/src/ && mkdir -p build && cd build && cmake $ENV{SDE}/p4studio/ -DCMAKE_INSTALL_PREFIX=$ENV{SDE}/install 
			 -DCMAKE_MODULE_PATH=$ENV{SDE}/cmake  -DP4_NAME=shc_real_time_sketch -DP4_PATH=${P4_SOURCE_DIR}/shc_real_time_sketch.p4) 
             # -DP4PPFLAGS="-Xp4c=--disable-parse-depth-limit")
set(P4_MAKE         cd ${P4_ROOT}/src/build/ && make)
set(P4_INSTALL      cd ${P4_ROOT}/src/build/ && make install)

ExternalProject_Add(P4
    PREFIX            ${P4_ROOT}
    SOURCE_DIR        ${P4_SOURCE_DIR}
    CONFIGURE_COMMAND ${P4_CONFIGURE}
    BUILD_COMMAND     ${P4_MAKE}
    INSTALL_COMMAND   ${P4_INSTALL}
)

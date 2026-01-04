include(ExternalProject)

set(GFLAGS_ROOT ${CMAKE_BINARY_DIR}/thirdparty/gflags)
set(GFLAGS_GIT_TAG      v2.2.2)
set(GFLAGS_GIT_URL      https://github.com/gflags/gflags.git)
set(GFLAGS_CONFIGURE    cd ${GFLAGS_ROOT}/src/GFLAGS && cmake . -DCMAKE_INSTALL_PREFIX=${GFLAGS_ROOT})
set(GFLAGS_MAKE         cd ${GFLAGS_ROOT}/src/GFLAGS && make)
set(GFLAGS_INSTALL      cd ${GFLAGS_ROOT}/src/GFLAGS && make install)

ExternalProject_Add(GFLAGS
        PREFIX            ${GFLAGS_ROOT}
        GIT_REPOSITORY    ${GFLAGS_GIT_URL}
        GIT_TAG           ${GFLAGS_GIT_TAG}
        CONFIGURE_COMMAND ${GFLAGS_CONFIGURE}
        BUILD_COMMAND     ${GFLAGS_MAKE}
        INSTALL_COMMAND   ${GFLAGS_INSTALL}
)

set(GFLAGS_LIB       ${GFLAGS_ROOT}/lib/libgflags.a)
set(GFLAGS_INCLUDE_DIR   ${GFLAGS_ROOT}/include)

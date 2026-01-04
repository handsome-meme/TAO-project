include(ExternalProject)

set(LIBFMT_ROOT ${CMAKE_BINARY_DIR}/thirdparty/fmtlib)
set(LIBFMT_GIT_TAG  7.0.0)
set(LIBFMT_GIT_URL      https://github.com/fmtlib/fmt.git)
set(LIBFMT_CONFIGURE    cd ${LIBFMT_ROOT}/src/LIBFMT && cmake . -DCMAKE_INSTALL_PREFIX=${LIBFMT_ROOT} -DFMT_TEST=)
set(LIBFMT_MAKE         cd ${LIBFMT_ROOT}/src/LIBFMT && make)
set(LIBFMT_INSTALL      cd ${LIBFMT_ROOT}/src/LIBFMT && make install)

ExternalProject_Add(LIBFMT
        PREFIX            ${LIBFMT_ROOT}
        GIT_REPOSITORY    ${LIBFMT_GIT_URL}
        GIT_TAG           ${LIBFMT_GIT_TAG}
        CONFIGURE_COMMAND ${LIBFMT_CONFIGURE}
        BUILD_COMMAND     ${LIBFMT_MAKE}
        INSTALL_COMMAND   ${LIBFMT_INSTALL}
)

set(LIBFMT_LIB       ${LIBFMT_ROOT}/lib/libfmt.a)
set(LIBFMT_INCLUDE_DIR   ${LIBFMT_ROOT}/include)

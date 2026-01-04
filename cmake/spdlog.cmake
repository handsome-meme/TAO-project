include(ExternalProject)

set(SPDLOG_ROOT ${CMAKE_BINARY_DIR}/thirdparty/spdlog)
set(SPDLOG_GIT_TAG      v1.7.0)
set(SPDLOG_GIT_URL      https://github.com/gabime/spdlog.git)
set(SPDLOG_CONFIGURE    cd ${SPDLOG_ROOT}/src/SPDLOG && cmake -D CMAKE_INSTALL_PREFIX=${SPDLOG_ROOT} .)
set(SPDLOG_MAKE         cd ${SPDLOG_ROOT}/src/SPDLOG && make)
set(SPDLOG_INSTALL      cd ${SPDLOG_ROOT}/src/SPDLOG && make install)

ExternalProject_Add(SPDLOG
        PREFIX            ${SPDLOG_ROOT}
        GIT_REPOSITORY    ${SPDLOG_GIT_URL}
        GIT_TAG           ${SPDLOG_GIT_TAG}
        CONFIGURE_COMMAND ${SPDLOG_CONFIGURE}
        BUILD_COMMAND     ${SPDLOG_MAKE}
        INSTALL_COMMAND   ${SPDLOG_INSTALL}
)

set(SPDLOG_LIB           ${SPDLOG_ROOT}/lib/libspdlog.a)
set(SPDLOG_INCLUDE_DIR   ${SPDLOG_ROOT}/include)

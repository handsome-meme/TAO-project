# Add an static lib
add_library(common STATIC EXCLUDE_FROM_ALL
    ${CMAKE_CURRENT_SOURCE_DIR}/ipaddress.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ipaddresses.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/ipprefix.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lpm_trie.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tokenize.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/log.cpp
)

add_dependencies(common GFLAGS SPDLOG)

target_include_directories(common PUBLIC 
    ${SPDLOG_INCLUDE_DIR}
    ${GFLAGS_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(common PUBLIC ${SPDLOG_LIB}
    ${GFLAGS_LIB}
)

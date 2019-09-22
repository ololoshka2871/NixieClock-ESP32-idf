set(jsl_esp32_http_base ${COMPONENT_DIR}/server)
set(jsl_esp32_http_utils ${COMPONENT_DIR}/utils)
set(jsl_esp32_json ${COMPONENT_DIR}/json)

set(src_to_patch jsl-http.cpp)
file(READ ${jsl_esp32_http_base}/${src_to_patch} jsl_http_cpp)
    string(REPLACE "while(ret == ERR_OK);"
        "while(ret != ERR_MEM && ret != ERR_BUF && ret != ERR_VAL && ret != ERR_IF && ret != ERR_ARG);"
        jsl_http_cpp "${jsl_http_cpp}")
    string(REPLACE "vTaskDelay(pdMS_TO_TICKS(1))"
        "vTaskDelay(1)"
        jsl_http_cpp "${jsl_http_cpp}")

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${src_to_patch} "${jsl_http_cpp}")

set(jsl_esp32_http_src
	${jsl_esp32_http_base}/jsl-common.cpp
#	${jsl_esp32_http_base}/jsl-http.cpp
        ${CMAKE_CURRENT_BINARY_DIR}/jsl-http.cpp
	${jsl_esp32_http_base}/jsl-router.cpp
        ${jsl_esp32_http_utils}/jsl-str.cpp
        ${jsl_esp32_json}/jsl-data.cpp
        ${jsl_esp32_json}/jsl-parser.cpp
)

idf_component_register(SRCS ${jsl_esp32_http_src}
                       INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
                       REQUIRES
                        esp32
                        log
)

target_link_libraries(${COMPONENT_LIB} PUBLIC ${LIBC})

target_include_directories(${COMPONENT_LIB}
    PRIVATE
        ${jsl_esp32_http_base}
)


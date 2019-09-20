set(jsl_esp32_http_base ${COMPONENT_DIR}/server)
set(jsl_esp32_http_utils ${COMPONENT_DIR}/utils)
set(jsl_esp32_json ${COMPONENT_DIR}/json)

set(jsl_esp32_http_src
	${jsl_esp32_http_base}/jsl-common.cpp
	${jsl_esp32_http_base}/jsl-http.cpp
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

#target_compile_options(${COMPONENT_LIB}
#    PRIVATE
#        -Wno-unused-variable
#)

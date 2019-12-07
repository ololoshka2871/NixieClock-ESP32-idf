set(socketstream_dir ${COMPONENT_DIR}/socketstream/src)

set(socketstream_dir_patched ${CMAKE_CURRENT_BINARY_DIR})

file(COPY ${socketstream_dir}/basic_socketbuf.hh DESTINATION ${socketstream_dir_patched})

file(READ ${socketstream_dir}/impl/basic_socketbuf.cc basic_socketbuf_cc)
string(REPLACE "try" " " basic_socketbuf_cc "${basic_socketbuf_cc}")
string(REPLACE "catch (...) {}" " " basic_socketbuf_cc "${basic_socketbuf_cc}")
file(WRITE ${socketstream_dir_patched}/impl/basic_socketbuf.cc "${basic_socketbuf_cc}")

idf_component_register(SRCS ${socketstream_dir}
                       INCLUDE_DIRS ${socketstream_dir_patched} ${socketstream_dir}
                       REQUIRES
)

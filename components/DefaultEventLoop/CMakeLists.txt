set(DefaultEventLoop_src
    DefaultEventLoop.cpp
)

idf_component_register(SRCS ${DefaultEventLoop_src}
                       INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
                       REQUIRES
                        esp32
)

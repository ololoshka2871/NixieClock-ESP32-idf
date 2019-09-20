set(VSensors_src
    VSensors.cpp
)

idf_component_register(SRCS ${VSensors_src}
                       INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
                       REQUIRES
                        esp32
)

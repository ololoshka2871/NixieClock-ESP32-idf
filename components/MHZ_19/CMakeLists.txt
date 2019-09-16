set(MHZ_19_src
    MHZ19.cpp
)

set(REQUIRES esp32 log)

idf_component_register(SRCS ${MHZ_19_src}
                       INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR})


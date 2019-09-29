set(Timer_src
    Timer.cpp
)

idf_component_register(SRCS ${Timer_src}
                       INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
                       REQUIRES
                        esp32
)

#target_compile_options(${COMPONENT_LIB}
#    PRIVATE
#        -Wno-unused-variable
#)

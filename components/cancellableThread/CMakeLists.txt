set(Timer_src
    cancellableThread.cpp
)

idf_component_register(SRCS ${Timer_src}
                       INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
                       REQUIRES
                        freertos
)

component_compile_options(-std=c++17)

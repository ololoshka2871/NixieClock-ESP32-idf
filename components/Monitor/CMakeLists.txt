set(Monitor_src
    monitor.cpp
)

idf_component_register(SRCS ${Monitor_src}
                       INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
                       REQUIRES
                        log
                        cancellableThread
)

component_compile_options(-std=c++17)

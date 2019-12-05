set(NetAdapter_src
    NetAdapter.cpp
    Network.cpp
    SoftAP.cpp
    STA.cpp
)

idf_component_register(SRCS ${NetAdapter_src}
                       INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
                       REQUIRES
                        esp32
                        log
                        nvs_flash
                        DefaultEventLoop
)

component_compile_options(-std=c++17)

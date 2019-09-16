set(VSensors_src
    VSensors.cpp
)

set(REQUIRES esp32 esp_adc_cal)

idf_component_register(SRCS ${VSensors_src}
                       INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR})


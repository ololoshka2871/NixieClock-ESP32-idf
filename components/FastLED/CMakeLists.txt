set(FastLED_base ${COMPONENT_DIR}/FastLED_dist)
set(FastLED_platform_dir ${FastLED_base}/platforms/esp/32)

set(FastLED_src
    ${FastLED_base}/bitswap.cpp
    ${FastLED_base}/bitswap.cpp
    ${FastLED_base}/colorpalettes.cpp
    ${FastLED_base}/colorutils.cpp
    ${FastLED_base}/FastLED.cpp
    ${FastLED_base}/hsv2rgb.cpp
    ${FastLED_base}/lib8tion.cpp
    ${FastLED_base}/noise.cpp
    ${FastLED_base}/platforms.cpp
    ${FastLED_base}/power_mgt.cpp
    ${FastLED_base}/wiring.cpp

    arduino-calls.cpp
)

set(REQUIRES
        esp32
        freertos
)

set(src_to_patch FastLED.h)
set(orig_src_to_patch FastLED.h.orig)
if (EXISTS ${FastLED_base}/${src_to_patch})
    file(RENAME ${FastLED_base}/${src_to_patch} ${FastLED_base}/${orig_src_to_patch})
endif()


file(READ ${FastLED_base}/${orig_src_to_patch} FastLED_h)
    string(REPLACE "#include <stdint.h>" "#include <stdint.h>
#include <FakeArduino.h>

extern \"C\" unsigned long micros();
extern \"C\" unsigned long millis();"  FastLED_h "${FastLED_h}")

    #string(REPLACE " {}" " {};" FastLED_h "${FastLED_h}")

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${src_to_patch} "${FastLED_h}")

idf_component_register(SRCS ${FastLED_src}
                       INCLUDE_DIRS
                        ${CMAKE_CURRENT_BINARY_DIR}
                        ${COMPONENT_DIR}/include
                        ${FastLED_base}
)

target_compile_definitions(${COMPONENT_LIB}
    PUBLIC
        -DFASTLED_USE_PROGMEM=0
        -DFASTLED_HAS_CLOCKLESS
        -DFASTLED_RMT_BUILTIN_DRIVER=1
        -DCLOCKLESS_FREQUENCY=160000000
        -DESP32
)

target_compile_options(${COMPONENT_LIB}
    PUBLIC
        $<$<COMPILE_LANGUAGE:CXX>:-Wno-class-memaccess -Wno-register>
)

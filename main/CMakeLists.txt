set(sources
    main.cpp
    Nixie.cpp
    RTC.cpp
    String_format.cpp
)

idf_component_register(SRCS ${sources}
                    INCLUDE_DIRS ".")

component_compile_options(-std=c++17)

########################################################################################################################

string(TIMESTAMP  CLOCK_START_YEAR "%Y")
string(TIMESTAMP  CLOCK_START_MON "%m")
string(REGEX REPLACE "^0" "" CLOCK_START_MON "${CLOCK_START_MON}")
string(TIMESTAMP  CLOCK_START_DAY "%d")
string(REGEX REPLACE "^0" "" CLOCK_START_DAY "${CLOCK_START_DAY}")
string(TIMESTAMP  CLOCK_START_HOUR "%H")
string(REGEX REPLACE "^0" "" CLOCK_START_HOUR "${CLOCK_START_HOUR}")
string(TIMESTAMP  CLOCK_START_MIN "%M")
string(REGEX REPLACE "^0" "" CLOCK_START_MIN "${CLOCK_START_MIN}")
string(TIMESTAMP  CLOCK_START_SEC "%S")
string(REGEX REPLACE "^0" "" CLOCK_START_SEC "${CLOCK_START_SEC}")

target_compile_definitions(${COMPONENT_LIB}
    PRIVATE
        -DCLOCK_START_YEAR=${CLOCK_START_YEAR}
        -DCLOCK_START_MON=${CLOCK_START_MON}
        -DCLOCK_START_DAY=${CLOCK_START_DAY}
        -DCLOCK_START_HOUR=${CLOCK_START_HOUR}
        -DCLOCK_START_MIN=${CLOCK_START_MIN}
        -DCLOCK_START_SEC=${CLOCK_START_SEC}
)

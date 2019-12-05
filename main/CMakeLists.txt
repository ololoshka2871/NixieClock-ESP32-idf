set(sources
    main.cpp
    Nixie.cpp
    RTC.cpp
    String_format.cpp

    InterfaceButton.cpp

    servers/mDNSServer.cpp
    servers/HttpServer.cpp
    servers/VirtuinoJsonServer.cpp

    sensors/TemperatureSensor.cpp
    sensors/CO2Sensor.cpp

    GUI/GUI.cpp

    GUI/GUIStates/AbstractGUIState.cpp
    GUI/GUIStates/InitialTransition.cpp
    GUI/GUIStates/ClockState.cpp
    GUI/GUIStates/CO2Monitor.cpp
    GUI/GUIStates/TemperatureMonitor.cpp
    GUI/GUIStates/DateDisplayer.cpp

    GUI/GUIStates/QuickTransition.cpp
    GUI/GUIStates/ProgressTransition.cpp

    GUI/Animations/AbstractLedAnimation.cpp
    GUI/Animations/ProgressLedAnimation.cpp
)

idf_component_register(SRCS ${sources}
                    INCLUDE_DIRS "."
                    REQUIRES
                        log
                        mdns
                        freertos

                        DynamicIndication
                        ds1307
                        ds18x20
                        FastLED
                        MHZ_19
                        VoltageSensors
                        jsl-esp32
                        Timer
                        Monitor
                        DefaultEventLoop
                        esp32-wifi-manager
)

component_compile_options(-std=c++17)

target_compile_definitions(${COMPONENT_LIB}
    PRIVATE
        #-DTEST_MODE
)

target_link_libraries(${COMPONENT_LIB} PUBLIC ${LIBC})

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

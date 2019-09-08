set(sources
    main.cpp
    Nixie.cpp
    RTC.cpp
    String_format.cpp
)

idf_component_register(SRCS ${sources}
                    INCLUDE_DIRS ".")

component_compile_options(-std=c++17)

set(dynamic_indication_src
    Controller.h
    DataBus.h
    NativeState.h

    DataPolycy/ic74141ParralelPolcy.h
    EncodePolicy/Encoder.h
    SelectorPolicy/Selector.h
)

idf_component_register(#SRCS ${dynamic_indication_src}
                       INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
                       REQUIRES
                        esp32
)

#target_compile_options(${COMPONENT_LIB}
#    PRIVATE
#        -Wno-unused-variable
#)

# Map the Kconfig "Connection" choice (common/Kconfig.variant) to the source-level CONNECT_VIA_*
# macro that both the Arduino and ESP-IDF builds use. Include this from an example's
# main/CMakeLists.txt *after* idf_component_register() (it needs ${COMPONENT_LIB}). Only the
# PlotToSerial example needs a variant; other examples do not include this file.
# Default (nothing selected): UnitUltraSonicI2C.
if(CONFIG_EXAMPLE_CONNECT_VIA_GPIO)
    set(M5UNIT_VARIANT CONNECT_VIA_GPIO)
else()
    set(M5UNIT_VARIANT CONNECT_VIA_I2C)
endif()
target_compile_definitions(${COMPONENT_LIB} PRIVATE ${M5UNIT_VARIANT})

# Dependencies (M5Unit-DISTANCE, M5UnitUnified, M5Unified, ...) are pulled in via
# main/idf_component.yml and auto-added as requirements by the component manager.
idf_component_register(
    SRCS "PlotToSerial.cpp"
    INCLUDE_DIRS ".")

# The connection (UnitUltraSonicI2C / UnitUltraSonicIO) is selected via Kconfig (main/Kconfig.projbuild ->
# common/Kconfig.variant). Include the CONFIG_* -> CONNECT_VIA_* mapping after the register call.
include("${CMAKE_CURRENT_LIST_DIR}/../../../common/variant.cmake")

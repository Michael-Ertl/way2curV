cmake_minimum_required(VERSION 3.20)
 
#set(CMAKE_C_COMPILER gcc)
#set(CMAKE_CXX_COMPILER g++)
#set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(way2curV VERSION 0.1.0 LANGUAGES C CXX)

include(GNUInstallDirs)

find_program(WAYLAND_SCANNER_EXECUTABLE NAMES wayland-scanner REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(WAYLAND_CLIENT REQUIRED IMPORTED_TARGET wayland-client)
pkg_check_modules(WAYLAND_PROTOCOLS REQUIRED wayland-protocols)
pkg_check_modules(WLR_PROTOCOLS REQUIRED wlr-protocols)

pkg_get_variable(WAYLAND_PROTOCOLS_DIR wayland-protocols pkgdatadir)
pkg_get_variable(WLR_PROTOCOLS_DIR wlr-protocols pkgdatadir)

set(LAYER_SHELL_XML
    "${WLR_PROTOCOLS_DIR}/unstable/wlr-layer-shell-unstable-v1.xml"
)
set(XDG_SHELL_XML
    "${WAYLAND_PROTOCOLS_DIR}/stable/xdg-shell/xdg-shell.xml"
)
set(GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")

# Generate protocol files at build time, and regenerate when their XML changes.
add_custom_command(
    OUTPUT
        "${GENERATED_DIR}/wlr-layer-shell-unstable-v1-client-protocol.h"
        "${GENERATED_DIR}/wlr-layer-shell-unstable-v1-protocol.c"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${GENERATED_DIR}"
    COMMAND "${WAYLAND_SCANNER_EXECUTABLE}" client-header
        "${LAYER_SHELL_XML}"
        "${GENERATED_DIR}/wlr-layer-shell-unstable-v1-client-protocol.h"
    COMMAND "${WAYLAND_SCANNER_EXECUTABLE}" private-code
        "${LAYER_SHELL_XML}"
        "${GENERATED_DIR}/wlr-layer-shell-unstable-v1-protocol.c"
    DEPENDS "${LAYER_SHELL_XML}"
    COMMENT "Generating layer-shell protocol files"
    VERBATIM
)

add_custom_command(
    OUTPUT "${GENERATED_DIR}/xdg-shell-protocol.c"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${GENERATED_DIR}"
    COMMAND "${WAYLAND_SCANNER_EXECUTABLE}" private-code
        "${XDG_SHELL_XML}"
        "${GENERATED_DIR}/xdg-shell-protocol.c"
    DEPENDS "${XDG_SHELL_XML}"
    COMMENT "Generating XDG shell protocol code"
    VERBATIM
)


file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

file(GLOB_RECURSE APP_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
)

add_executable(way2curV
    ${APP_SOURCES}
    ${APP_HEADERS}
    "${GENERATED_DIR}/wlr-layer-shell-unstable-v1-client-protocol.h"
    "${GENERATED_DIR}/wlr-layer-shell-unstable-v1-protocol.c"
    "${GENERATED_DIR}/xdg-shell-protocol.c"
)

target_compile_features(way2curV PRIVATE cxx_std_20)
target_compile_definitions(way2curV PRIVATE _POSIX_C_SOURCE=200112L)

target_include_directories(way2curV PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${GENERATED_DIR}"
)

target_link_libraries(way2curV PRIVATE PkgConfig::WAYLAND_CLIENT)



install(TARGETS way2curV
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES config.conf
    DESTINATION "${CMAKE_INSTALL_DATADIR}/way2curV"
    RENAME "default.conf"
)

target_compile_definitions(way2curV PRIVATE
    WAY2CURV_DEFAULT_CONFIG="${CMAKE_INSTALL_FULL_DATADIR}/way2curV/default.conf"
)
install(FILES README.md LICENSE
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
)

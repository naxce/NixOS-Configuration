cmake_minimum_required(VERSION 3.16)
project(zenith VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_compile_options(
    -O3
    -march=native
    -pipe
    -fno-plt
    -fstack-protector-strong
    -Wall
    -Wextra
    -Wno-unused-parameter
    -D_FORTIFY_SOURCE=2
)

find_package(PkgConfig REQUIRED)

pkg_check_modules(WAYLAND_SERVER REQUIRED wayland-server)
pkg_check_modules(EGL REQUIRED egl)
pkg_check_modules(GLES2 REQUIRED glesv2)
pkg_check_modules(GBM REQUIRED gbm)
pkg_check_modules(LIBDRM REQUIRED libdrm)
pkg_check_modules(LIBINPUT REQUIRED libinput)
pkg_check_modules(LIBUDEV REQUIRED libudev)
pkg_check_modules(CAIRO REQUIRED cairo)
pkg_check_modules(PIXMAN REQUIRED pixman-1)
pkg_check_modules(FONTCONFIG REQUIRED fontconfig)

find_package(Threads REQUIRED)

set(SOURCES
    src/zenith.c
    src/config.c
    src/output.c
    src/window.c
    src/input.c
    src/keybinds.c
    src/taskbar.c
    src/desktop.c
    src/launcher.c
    src/settings_panel.c
    src/monitor_settings.c
    protocol/xdg-shell-protocol.c
)

set(PROTOCOL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/protocol)

add_custom_command(
    OUTPUT ${PROTOCOL_DIR}/xdg-shell-server-protocol.h
    COMMAND wayland-scanner server-header
            ${CMAKE_CURRENT_SOURCE_DIR}/protocol/xdg-shell.xml
            ${PROTOCOL_DIR}/xdg-shell-server-protocol.h
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/protocol/xdg-shell.xml
)

add_custom_command(
    OUTPUT ${PROTOCOL_DIR}/xdg-shell-protocol.c
    COMMAND wayland-scanner private-code
            ${CMAKE_CURRENT_SOURCE_DIR}/protocol/xdg-shell.xml
            ${PROTOCOL_DIR}/xdg-shell-protocol.c
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/protocol/xdg-shell.xml
)

add_custom_target(protocols DEPENDS
    ${PROTOCOL_DIR}/xdg-shell-server-protocol.h
    ${PROTOCOL_DIR}/xdg-shell-protocol.c
)

add_executable(zenith ${SOURCES})
add_dependencies(zenith protocols)

target_include_directories(zenith PRIVATE
    include
    protocol
    ${WAYLAND_SERVER_INCLUDE_DIRS}
    ${EGL_INCLUDE_DIRS}
    ${GLES2_INCLUDE_DIRS}
    ${GBM_INCLUDE_DIRS}
    ${LIBDRM_INCLUDE_DIRS}
    ${LIBINPUT_INCLUDE_DIRS}
    ${LIBUDEV_INCLUDE_DIRS}
    ${CAIRO_INCLUDE_DIRS}
    ${PIXMAN_INCLUDE_DIRS}
    ${FONTCONFIG_INCLUDE_DIRS}
)

target_link_libraries(zenith PRIVATE
    ${WAYLAND_SERVER_LIBRARIES}
    ${EGL_LIBRARIES}
    ${GLES2_LIBRARIES}
    ${GBM_LIBRARIES}
    ${LIBDRM_LIBRARIES}
    ${LIBINPUT_LIBRARIES}
    ${LIBUDEV_LIBRARIES}
    ${CAIRO_LIBRARIES}
    ${PIXMAN_LIBRARIES}
    ${FONTCONFIG_LIBRARIES}
    Threads::Threads
    m
)

target_compile_options(zenith PRIVATE
    ${WAYLAND_SERVER_CFLAGS_OTHER}
    ${EGL_CFLAGS_OTHER}
    ${GLES2_CFLAGS_OTHER}
    ${GBM_CFLAGS_OTHER}
    ${LIBDRM_CFLAGS_OTHER}
    ${LIBINPUT_CFLAGS_OTHER}
    ${LIBUDEV_CFLAGS_OTHER}
    ${CAIRO_CFLAGS_OTHER}
    ${PIXMAN_CFLAGS_OTHER}
    ${FONTCONFIG_CFLAGS_OTHER}
)

install(TARGETS zenith RUNTIME DESTINATION bin)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/nixos/zenith.desktop
    DESTINATION share/wayland-sessions)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/nixos/zenith-sddm.desktop
    DESTINATION share/xsessions)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/config/
    DESTINATION share/zenith
    FILES_MATCHING PATTERN "*.conf" PATTERN "*.sh")

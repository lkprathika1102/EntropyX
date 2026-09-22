# Copyright 2026 NXP
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.22.0)

cmake_path(APPEND conf_file_path ${CMAKE_CURRENT_LIST_DIR} ${board} prj.conf)
set(CONF_FILE ${conf_file_path} ${CONF_FILE})
include(${SdkRootDirPath}/cmake/extension/mcux.cmake)

project(EntropyX LANGUAGES C CXX ASM PROJECT_BOARD_PORT_PATH ${board_root}/${board}/demo_apps/hello_world)

include(${SdkRootDirPath}/CMakeLists.txt)
include(${board}/board_files.cmake)

# --- CRITICAL: EVERY .C FILE MUST BE LISTED HERE ---
mcux_add_source(
    SOURCES main.c
            gy87.c
            ssd1306.c        # <--- DOUBLE CHECK THIS LINE EXISTS
            entropy_math.c
            crypto_hw.c
)

mcux_convert_binary(BINARY ${APPLICATION_BINARY_DIR}/${MCUX_SDK_PROJECT_NAME}.bin)
mcux_add_config_mex_path(PATH ${board})
mcux_add_include(INCLUDES .)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)
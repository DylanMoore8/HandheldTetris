cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME TetrisHandheld)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
    
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    Core/App/Src/game/game.c
    Core/App/Src/game/state_stack.c
    Core/App/Src/game/session.c
    Core/App/Src/game/title.c
    Core/App/Src/game/globals.c
    Core/App/Src/game/rng.c
    Core/App/Src/game/sprites.c
    Core/App/Src/screen.c
    Core/App/Src/input.c
    Core/App/Src/radio.c
    Drivers/SX1278/SX1278/driver/SX1278.c
    Drivers/SX1278/SX1278/driver/SX1278_hw.c
    Drivers/ssd1306/ssd1306.c
    # HAL TIM driver: needed for the TIM2 HAL timebase (CubeMX omits it)
    # Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c
    # Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex.c
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    Core/App/Inc
    Core/App/Inc/game
    Drivers/SX1278/SX1278/driver
    Drivers/ssd1306
)

# load defs from config file
file(READ "${CMAKE_SOURCE_DIR}/config.json" JSON_CONTENT)
# get values
string(JSON id GET "${JSON_CONTENT}" "id")
string(JSON useName GET "${JSON_CONTENT}" "useName")

if (useName)
    set(useName true)
    string(JSON displayName GET "${JSON_CONTENT}" "name")
    target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
    DISPLAY_NAME="${displayName}"
    )
else()
    set(useName false)
endif()

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
    BOARD_ID=${id}
    USE_NAME=${useName}
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)

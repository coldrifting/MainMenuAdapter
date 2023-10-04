cmake_minimum_required(VERSION 3.21)
message("Using toolchain file ${CMAKE_TOOLCHAIN_FILE}.")

project(
        MainMenuAdapter
        VERSION 1.0.0
        DESCRIPTION "Automatically converts skyrim main menu replacers to work with the Main Menu Replacer"
        LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(nifly CONFIG REQUIRED)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE nifly)

set(BIT7Z_AUTO_FORMAT Bool "On")
add_subdirectory( ${CMAKE_SOURCE_DIR}/bit7z )
target_link_libraries( ${PROJECT_NAME} PRIVATE bit7z )

add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_SOURCE_DIR}/lib/7z.dll
                ${CMAKE_CURRENT_BINARY_DIR}/7z.dll)
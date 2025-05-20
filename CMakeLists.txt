cmake_minimum_required(VERSION 3.10)
project(Sudoku)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(SDL2_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/SDL2/cmake")
set(SDL2_PATH "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/SDL2")


set(SDL2_TTF_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/SDL2_ttf/cmake")
set(SDL2_TTF_PATH "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/SDL2_ttf")


find_package(SDL2 REQUIRED)


if(EXISTS "${SDL2_TTF_DIR}/SDL2_ttfConfig.cmake")
    find_package(SDL2_ttf REQUIRED)
    set(SDL2_TTF_LIBRARIES SDL2_ttf::SDL2_ttf)
else()

    include_directories(${SDL2_TTF_PATH}/include)
    if(WIN32)

        if(CMAKE_SIZEOF_VOID_P EQUAL 8)

            set(SDL2_TTF_LIBRARIES "${SDL2_TTF_PATH}/lib/x64/SDL2_ttf.lib")
            set(SDL2_TTF_DLL "${SDL2_TTF_PATH}/lib/x64/SDL2_ttf.dll")
        else()
            set(SDL2_TTF_LIBRARIES "${SDL2_TTF_PATH}/lib/x86/SDL2_ttf.lib")
            set(SDL2_TTF_DLL "${SDL2_TTF_PATH}/lib/x86/SDL2_ttf.dll")
        endif()
    else()
        set(SDL2_TTF_LIBRARIES "${SDL2_TTF_PATH}/lib/libSDL2_ttf.a")
    endif()
endif()

include_directories(include)
include_directories(${SDL2_PATH}/include)
include_directories(${SDL2_TTF_PATH}/include)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES} ${SDL2_TTF_LIBRARIES})

if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL2_PATH}/lib/x64/SDL2.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
    
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL2_TTF_PATH}/lib/x64/SDL2_ttf.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif() 
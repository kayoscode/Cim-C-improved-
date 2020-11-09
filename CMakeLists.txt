cmake_minimum_required(VERSION 3.18)

SET(PROJ_NAME Emulator)

add_subdirectory(Logger)

project(${PROJ_NAME})

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:50000000")
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SRC 
    "src/*.cpp"
    "include/*.h"
    "Logger.cpp"
)

file(GLOB ASSEMBLER_SRC
    "Assembler/src/*.cpp"
    "Assembler/include/*.h"
    "Logger.cpp"
)

file(GLOB COMPILER_SRC
    "Compiler/src/*.cpp"
    "Compiler/include/*.h"
    "Logger.cpp"
)

add_executable(${PROJ_NAME}
    "${SRC}"
)

add_executable(Assembler
    "${ASSEMBLER_SRC}"
)

add_executable(Compiler
    "${COMPILER_SRC}"
)

target_link_libraries(${PROJ_NAME}
    bylog
)

target_link_libraries(Assembler
    bylog
)

target_link_libraries(Compiler
    bylog
)

target_include_directories("${PROJ_NAME}" PRIVATE ${PROJECT_SOURCE_DIR}/./include)
target_include_directories("${PROJ_NAME}" PRIVATE ${PROJECT_SOURCE_DIR}/Assembler/include)

target_include_directories(Assembler PRIVATE ${PROJECT_SOURCE_DIR}/Assembler/include)
target_include_directories(Assembler PRIVATE ${PROJECT_SOURCE_DIR}/./include)

target_include_directories(Compiler PRIVATE ${PROJECT_SOURCE_DIR}/Compiler/include)
target_include_directories(Compiler PRIVATE ${PROJECT_SOURCE_DIR}/./include)
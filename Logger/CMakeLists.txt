cmake_minimum_required(VERSION 3.8)

project("bylog")

file(GLOB SCR
    "*.h"
    "*.cpp"
)

add_library(bylog STATIC
    "${SCR}"
)

target_include_directories(bylog PRIVATE ${PROJECT_SOURCE_DIR}/./include)

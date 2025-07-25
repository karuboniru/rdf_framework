cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
project(nuanalysis)
include(FetchContent)
find_package(nlohmann_json)
if(NOT nlohmann_json_FOUND)
    FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
    FetchContent_MakeAvailable(json)
endif()
set(CMAKE_CXX_STANDARD 20)

find_package(ROOT REQUIRED)
find_package(Boost)
include_directories(
    ${ROOT_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -O3 -g3 -ggdb -Wall -Wextra -Wpedantic")
add_subdirectory(example)

file(GLOB SOURCES "src/*.cxx")
add_library(nuanalysis SHARED ${SOURCES})

add_executable(main main.cxx)
# set_property(TARGET main PROPERTY ENABLE_EXPORTS ON)
target_link_libraries(main nuanalysis ROOT::ROOTDataFrame nlohmann_json::nlohmann_json ${CMAKE_DL_LIBS})
install(TARGETS main DESTINATION bin)

if(Boost_FOUND)
    add_executable(gibuu_final_2_root gibuu_final_2_root.cxx)
    target_link_libraries(gibuu_final_2_root ROOT::Tree ${CMAKE_DL_LIBS})
    install(TARGETS gibuu_final_2_root DESTINATION bin)
endif()

add_executable(merge_plot merge_plot.cxx)
target_link_libraries(merge_plot ROOT::Tree ROOT::Hist ROOT::Gpad nlohmann_json::nlohmann_json ${CMAKE_DL_LIBS})
install(TARGETS merge_plot DESTINATION bin)

cmake_minimum_required(VERSION 3.9 FATAL_ERROR)
project(nuanalysis)
# set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
set(CMAKE_CXX_STANDARD 17)
include(FetchContent)
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
FetchContent_MakeAvailable(json)
find_package(ROOT REQUIRED)
include("${ROOT_USE_FILE}")
include_directories(
    ${ROOT_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -O3 -march=native -Wall -Wextra -Wpedantic")
add_subdirectory(example)

add_executable(main main.cxx)
set_property(TARGET main PROPERTY ENABLE_EXPORTS ON)
target_link_libraries(main ROOT::ROOTDataFrame nlohmann_json::nlohmann_json ${CMAKE_DL_LIBS})
install(TARGETS main DESTINATION bin)

add_executable(gibuu_final_2_root gibuu_final_2_root.cxx)
target_link_libraries(gibuu_final_2_root ROOT::Tree ${CMAKE_DL_LIBS})
install(TARGETS gibuu_final_2_root DESTINATION bin)

add_executable(merge_plot merge_plot.cxx)
target_compile_options(merge_plot PRIVATE -fconcepts)
target_link_libraries(merge_plot ROOT::Tree  ROOT::Hist ROOT::Gpad nlohmann_json::nlohmann_json ${CMAKE_DL_LIBS})
install(TARGETS merge_plot DESTINATION bin)
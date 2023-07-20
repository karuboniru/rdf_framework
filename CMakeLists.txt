cmake_minimum_required(VERSION 3.9 FATAL_ERROR)
project(nuanalysis)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

set(CMAKE_CXX_STANDARD 17)
add_compile_options(-fconcepts)
include(FetchContent)
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
FetchContent_MakeAvailable(json)
find_package(ROOT REQUIRED)
include("${ROOT_USE_FILE}")
include_directories(
    ${ROOT_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/example/include
)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -Wall -Wextra -Wpedantic")

add_library(genie_general OBJECT example/genie_general.cxx)

add_library(muon_p SHARED example/muon_p.cxx)
add_library(nuwroTKI SHARED example/nuwroTKI.cxx)
add_library(nuwrotoy SHARED example/nuwrotoy.cxx)
add_library(genie_kaon SHARED example/genie_kaon.cxx)
add_library(genie_pion SHARED example/genie_pion.cxx)
add_library(genie_proton SHARED example/genie_proton.cxx)
add_library(genieTKI SHARED example/genieTKI.cxx)
target_link_libraries(muon_p ROOT::Physics ROOT::ROOTDataFrame)
target_link_libraries(nuwroTKI ROOT::Physics ROOT::ROOTDataFrame)
target_link_libraries(nuwrotoy ROOT::Physics ROOT::ROOTDataFrame)
target_link_libraries(genie_kaon ROOT::Physics ROOT::ROOTDataFrame genie_general)
target_link_libraries(genie_pion ROOT::Physics ROOT::ROOTDataFrame genie_general)
target_link_libraries(genie_proton ROOT::Physics ROOT::ROOTDataFrame genie_general)
target_link_libraries(genieTKI ROOT::Physics ROOT::ROOTDataFrame genie_general)



add_executable(main main.cxx)
target_link_libraries(main ROOT::ROOTDataFrame nlohmann_json::nlohmann_json ${CMAKE_DL_LIBS})
install(TARGETS main DESTINATION bin)

add_executable(gibuu_final_2_root gibuu_final_2_root.cxx)
target_link_libraries(gibuu_final_2_root ROOT::Tree ${CMAKE_DL_LIBS})
install(TARGETS gibuu_final_2_root DESTINATION bin)

add_executable(merge_plot merge_plot.cxx)
target_link_libraries(merge_plot ROOT::Tree  ROOT::Hist ROOT::Gpad nlohmann_json::nlohmann_json ${CMAKE_DL_LIBS})
install(TARGETS merge_plot DESTINATION bin)
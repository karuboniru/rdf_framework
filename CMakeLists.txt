cmake_minimum_required(VERSION 3.9 FATAL_ERROR)
project(nuanalysis)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

set(CMAKE_CXX_STANDARD 17)
find_package(nlohmann_json 3.2.0 REQUIRED)
find_package(ROOT REQUIRED)
include("${ROOT_USE_FILE}")
include_directories(
    ${ROOT_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -Wall -Wextra -Wpedantic")

add_library(muon_p SHARED example/muon_p.cxx)
add_library(nuwroTKI SHARED example/nuwroTKI.cxx)
add_library(nuwrotoy SHARED example/nuwrotoy.cxx)
add_library(genie_kaon SHARED example/genie_kaon.cxx)
target_link_libraries(muon_p ROOT::Physics ROOT::ROOTDataFrame)
target_link_libraries(nuwroTKI ROOT::Physics ROOT::ROOTDataFrame)
target_link_libraries(nuwrotoy ROOT::Physics ROOT::ROOTDataFrame)
target_link_libraries(genie_kaon ROOT::Physics ROOT::ROOTDataFrame)



add_executable(main main.cxx)
target_link_libraries(main ROOT::ROOTDataFrame nlohmann_json::nlohmann_json ${CMAKE_DL_LIBS})
install(TARGETS main DESTINATION bin)
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(tki_general OBJECT tki_general.cxx)
add_library(nuwroTKI MODULE nuwroTKI.cxx)
add_library(genieTKI MODULE genieTKI.cxx)
# target_link_libraries(muon_p ROOT::Physics ROOT::ROOTDataFrame)
target_link_libraries(nuwroTKI ROOT::Physics ROOT::ROOTDataFrame tki_general)
target_link_libraries(genieTKI ROOT::Physics ROOT::ROOTDataFrame genie_general tki_general)

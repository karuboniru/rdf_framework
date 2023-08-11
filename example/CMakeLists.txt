include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(genie_general OBJECT genie_general.cxx)
add_library(tki_general OBJECT tki_general.cxx)
add_library(muon_p SHARED muon_p.cxx)
add_library(nuwroTKI SHARED nuwroTKI.cxx)
add_library(genie_kaon SHARED genie_kaon.cxx)
add_library(genie_pion SHARED genie_pion.cxx)
add_library(genie_proton SHARED genie_proton.cxx)
add_library(genieTKI SHARED genieTKI.cxx)
target_link_libraries(muon_p ROOT::Physics ROOT::ROOTDataFrame)
target_link_libraries(nuwroTKI ROOT::Physics ROOT::ROOTDataFrame tki_general)
target_link_libraries(genie_kaon ROOT::Physics ROOT::ROOTDataFrame genie_general)
target_link_libraries(genie_pion ROOT::Physics ROOT::ROOTDataFrame genie_general)
target_link_libraries(genie_proton ROOT::Physics ROOT::ROOTDataFrame genie_general)
target_link_libraries(genieTKI ROOT::Physics ROOT::ROOTDataFrame genie_general tki_general)

add_subdirectory(JunoEDM)
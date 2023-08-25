include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)


add_library(genie_general OBJECT genie_general.cxx)
# add_library(muon_p SHARED muon_p.cxx)
# add_library(genie_kaon SHARED genie_kaon.cxx)
# add_library(genie_pion SHARED genie_pion.cxx)
# add_library(genie_proton SHARED genie_proton.cxx)
# target_link_libraries(muon_p ROOT::Physics ROOT::ROOTDataFrame)
# target_link_libraries(genie_kaon ROOT::Physics ROOT::ROOTDataFrame genie_general)
# target_link_libraries(genie_pion ROOT::Physics ROOT::ROOTDataFrame genie_general)
# target_link_libraries(genie_proton ROOT::Physics ROOT::ROOTDataFrame genie_general)

add_subdirectory(TKI)
add_subdirectory(GENIESingle)
add_subdirectory(JunoEDM)
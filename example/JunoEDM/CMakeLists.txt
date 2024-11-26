find_package(junosw)
if (junosw_FOUND)
    add_library(SimHeadTest MODULE SimHead.cxx)
    target_link_libraries(SimHeadTest 
                            junosw::EDMUtil 
                            junosw::EvtNavigator 
                            junosw::GenEvent 
                            junosw::SimEvent
                          )

    add_library(elec2hit MODULE elec2hit.cxx)
    target_link_libraries(elec2hit 
                            junosw::EDMUtil 
                            junosw::EvtNavigator 
                            junosw::ElecTruth 
                            junosw::ElecEvent
                            junosw::GenEvent 
                            junosw::SimEvent
                            ROOT::Physics
                          )
endif()
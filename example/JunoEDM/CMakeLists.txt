find_package(junosw)
add_library(SimHeadTest SHARED SimHead.cxx)
target_link_libraries(SimHeadTest junosw::EDMUtil junosw::EvtNavigator junosw::GenEvent junosw::SimEvent)

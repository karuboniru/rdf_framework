include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_library(genie2event OBJECT genie2event.cc)
add_library(nuwro2event OBJECT nuwro2event.cc)
add_library(general_define OBJECT general_define.cc)

add_library(generator_event SHARED event.cc)
target_link_libraries(generator_event
  ROOT::Physics ROOT::ROOTDataFrame
  genie2event
  nuwro2event
  general_define
)

add_library(nuwroevent SHARED IMPORTED)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)
add_library(nuwro_normalization OBJECT nuwro_normalization.cxx)
add_library(nuwro_xsec MODULE nuwro_xsec.cxx)
target_link_libraries(nuwro_xsec ROOT::Physics ROOT::ROOTDataFrame nuwro_normalization)



if(DEFINED ENV{NUWRO})
  set_target_properties(nuwroevent PROPERTIES IMPORTED_LOCATION $ENV{NUWRO}/bin/event1.so )
  add_library(nuwro_normalization_from_raw OBJECT nuwro_normalization_from_raw.cxx)
  target_include_directories(nuwro_normalization_from_raw PUBLIC $ENV{NUWRO}/src)
  add_library(nuwro_xsec_from_raw MODULE nuwro_xsec_from_raw.cxx)
  target_include_directories(nuwro_xsec_from_raw PUBLIC $ENV{NUWRO}/src)
  target_link_directories(nuwro_xsec_from_raw PUBLIC $ENV{NUWRO}/bin)
  target_link_libraries(nuwro_xsec_from_raw nuwroevent ROOT::Physics ROOT::ROOTDataFrame nuwro_normalization_from_raw)
endif()
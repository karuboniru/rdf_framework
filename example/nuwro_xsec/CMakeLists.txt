include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)
add_library(nuwro_normalization OBJECT nuwro_normalization.cxx)
add_library(nuwro_xsec SHARED nuwro_xsec.cxx)
target_link_libraries(nuwro_xsec ROOT::Physics ROOT::ROOTDataFrame nuwro_normalization)

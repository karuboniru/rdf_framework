add_library(geniesinglekin SHARED geniesinglekin.cxx)
target_link_libraries(geniesinglekin ROOT::Physics ROOT::ROOTDataFrame genie_general)
set (algo_sources
  algo.h algo.cc
  coding-options.h coding-options.cc
  ctb-qscale.h ctb-qscale.cc
  cb-split.h cb-split.cc
  cb-intrapartmode.h cb-intrapartmode.cc
  cb-interpartmode.h cb-interpartmode.cc
  cb-skip.h cb-skip.cc
  cb-intra-inter.h cb-intra-inter.cc
  cb-mergeindex.h cb-mergeindex.cc
  tb-split.h tb-split.cc
  tb-transform.h tb-transform.cc
  tb-intrapredmode.h tb-intrapredmode.cc
  tb-rateestim.h tb-rateestim.cc
  pb-mv.h pb-mv.cc
)

add_library(algo OBJECT ${algo_sources})
set(ALGO_OBJECTS $<TARGET_OBJECTS:algo> PARENT_SCOPE)

if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
  SET_TARGET_PROPERTIES(algo PROPERTIES COMPILE_FLAGS "-fPIC")
endif(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")

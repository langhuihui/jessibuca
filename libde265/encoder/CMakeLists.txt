set (encoder_sources 
  analyze.cc analyze.h
  encode.h encode.cc
  encoder-params.h encoder-params.cc
  encoder-context.h encoder-context.cc
  encpicbuf.h encpicbuf.cc
  sop.h sop.cc
)

add_library(encoder STATIC ${encoder_sources})

if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
  SET_TARGET_PROPERTIES(encoder PROPERTIES COMPILE_FLAGS "-fPIC")
endif(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")

add_subdirectory (algo)

target_link_libraries(encoder algo)

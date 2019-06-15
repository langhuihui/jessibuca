set (libde265_sources 
  bitstream.cc
  cabac.cc
  de265.cc
  deblock.cc
  decctx.cc
  nal-parser.cc
  nal-parser.h
  dpb.cc
  dpb.h
  image.cc
  intrapred.cc
  md5.cc
  nal.cc
  pps.cc
  transform.cc
  refpic.cc
  sao.cc
  scan.cc
  sei.cc
  slice.cc
  sps.cc
  util.cc
  vps.cc
  bitstream.h
  cabac.h
  deblock.h
  decctx.h
  image.h
  intrapred.h
  md5.h
  nal.h
  pps.h
  transform.h
  refpic.h
  sao.h
  scan.h
  sei.h
  slice.h
  sps.h
  util.h
  vps.h
  vui.h vui.cc
  motion.cc motion.h
  threads.cc threads.h
  visualize.cc visualize.h
  acceleration.h
  fallback.cc fallback.h fallback-motion.cc fallback-motion.h
  fallback-dct.h fallback-dct.cc
  quality.cc quality.h
  configparam.cc configparam.h
  image-io.h image-io.cc
  alloc_pool.h alloc_pool.cc
  en265.h en265.cc
  contextmodel.cc
)

if(MSVC)
  set (libde265_sources
    ${libde265_sources}
    ../extra/win32cond.c
    ../extra/win32cond.h
  )
endif()

add_definitions(-DLIBDE265_EXPORTS)

add_subdirectory (encoder)

if(SUPPORTS_SSE4_1)
  add_definitions(-DHAVE_SSE4_1)
  add_subdirectory (x86)
endif()

add_library(${LIBDE265_LIBRARY_NAME} SHARED ${libde265_sources} ${ENCODER_OBJECTS} ${X86_OBJECTS})
target_link_libraries(${LIBDE265_LIBRARY_NAME} ${CMAKE_THREAD_LIBS_INIT})

if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
  SET_TARGET_PROPERTIES(${LIBDE265_LIBRARY_NAME} PROPERTIES COMPILE_FLAGS "-fPIC")
endif(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")

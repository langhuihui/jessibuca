set (enc265_sources
  enc265.cc
  image-io-png.cc
)

if(MSVC)
  set (enc265_sources
    ${enc265_sources}
    ../extra/getopt.c
    ../extra/getopt_long.c
  )
endif()

add_executable (enc265 ${enc265_sources})

target_link_libraries (enc265 ${LIBDE265_LIBRARY_NAME})


bin_PROGRAMS = dec265 hdrcopy

AM_CPPFLAGS = -I../libde265

dec265_DEPENDENCIES = ../libde265/libde265.la
dec265_CXXFLAGS =
dec265_LDFLAGS =
dec265_LDADD = ../libde265/libde265.la -lstdc++
dec265_SOURCES = dec265.cc

hdrcopy_DEPENDENCIES = ../libde265/libde265.la
hdrcopy_CXXFLAGS =
hdrcopy_LDFLAGS =
hdrcopy_LDADD = ../libde265/libde265.la -lstdc++
hdrcopy_SOURCES = hdrcopy.cc

if HAVE_VIDEOGFX
  dec265_CXXFLAGS += $(VIDEOGFX_CFLAGS)
  dec265_LDFLAGS += $(VIDEOGFX_LIBS)
endif

if HAVE_SDL
  dec265_CXXFLAGS += $(SDL_CFLAGS)
  dec265_LDFLAGS += $(SDL_LIBS)
  dec265_SOURCES += sdl.cc sdl.hh
endif

if MINGW
  dec265_LDFLAGS += -static-libgcc -static-libstdc++
endif

EXTRA_DIST = Makefile.vc7 \
  CMakeLists.txt \
  ../extra/getopt.c \
  ../extra/getopt.h \
  ../extra/getopt_long.c

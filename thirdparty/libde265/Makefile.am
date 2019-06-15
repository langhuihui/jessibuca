## Makefile.am for libde265

SUBDIRS = libde265

ACLOCAL_AMFLAGS = -I m4

if ENABLE_DEC265
SUBDIRS+=dec265
endif

SUBDIRS+=enc265
SUBDIRS+=tools
SUBDIRS+=acceleration-speed

if ENABLE_SHERLOCK265
SUBDIRS+=sherlock265
endif

EXTRA_DIST = .travis.yml \
  autogen.sh \
  build.bat \
  m4/m4_ax_check_compile_flag.m4 \
  Makefile.vc7 \
  CMakeLists.txt \
  README.md \
  libde265.png \
  */COPYING

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libde265.pc

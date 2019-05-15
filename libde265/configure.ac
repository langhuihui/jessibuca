#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([libde265], [1.0.3], [farin@struktur.de])
AC_CONFIG_SRCDIR([libde265/de265.cc])
AC_CONFIG_HEADERS([config.h])

NUMERIC_VERSION=0x01000300 # Numeric representation of the version (A.B.C[.D] = 0xAABBCCDD)
AC_SUBST(NUMERIC_VERSION)

LIBDE265_CURRENT=0
LIBDE265_REVISION=10
LIBDE265_AGE=0

# ---------------------------------------------------------------------------

AC_CANONICAL_SYSTEM

AC_SUBST(LIBDE265_CURRENT)
AC_SUBST(LIBDE265_REVISION)
AC_SUBST(LIBDE265_AGE)

dnl Initialize libtool
LT_INIT
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AM_PROG_AS
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_GREP

# Initialize automake stuff
AM_INIT_AUTOMAKE

CFLAGS="$CFLAGS -std=c99"
CXXFLAGS="$CXXFLAGS -Werror=return-type -Werror=unused-result -Werror=reorder"
AX_CXX_COMPILE_STDCXX_11()

dnl Use -Wall if we have gcc.
changequote(,)dnl
if test "x$GCC" = "xyes"; then
  case " $CFLAGS " in
  *[\ \	]-Wall[\ \	]*) ;;
  *) CFLAGS="$CFLAGS -Wall" ;;
  esac
fi
changequote([,])dnl

dnl gl_VISIBILITY
dnl : In encoder branch, we still export all library symbols :
HAVE_VISIBILITY=0
AM_CONDITIONAL([HAVE_VISIBILITY], [test "x$HAVE_VISIBILITY" != "x0"])

# Checks for header files.
AC_CHECK_HEADERS([stdint.h stdlib.h string.h malloc.h signal.h setjmp.h stddef.h sys/time.h])

AC_LANG_PUSH(C++)
OLD_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CXXFLAGS"
AC_CHECK_HEADERS([cstdint])
CPPFLAGS="$OLD_CPPFLAGS"
AC_LANG_POP(C++)

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_SIZE_T
AC_TYPE_INT8_T
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_CHECK_TYPES([ptrdiff_t])
AC_C_INLINE

# Checks for library functions.
AC_CHECK_FUNCS([malloc memmove memset __malloc_hook memalign posix_memalign __mingw_aligned_malloc __mingw_aligned_free])

AC_SEARCH_LIBS([pow], [m])
AC_SEARCH_LIBS([sqrt], [m])
AC_SEARCH_LIBS([pthread_create], [pthread])

AC_CHECK_FUNCS([gettimeofday])
AC_CHECK_FUNCS([pow sqrt])
AC_CHECK_FUNCS([strchr strrchr])

AC_FUNC_ALLOCA
AC_FUNC_ERROR_AT_LINE
# Checking for malloc breaks building on ARM for us. A similar issue is described
# here: http://nerdland.net/unstumping-the-internet/malloc-has-not-been-declared/
# AC_FUNC_MALLOC
# AC_FUNC_REALLOC
AC_FUNC_MKTIME

AM_CONDITIONAL(MINGW, expr $host : '.*-mingw' >/dev/null 2>&1)

# Check if "__STRICT_ANSI__" is required.
AC_MSG_CHECKING([if __STRICT_ANSI__ is required])
AC_LANG_PUSH(C++)
AC_TRY_COMPILE([
#include <vector>
],[],[need_strict_ansi=no],[need_strict_ansi=yes]);
AC_LANG_POP(C++)
if eval "test x$need_strict_ansi = xyes"; then
  CFLAGS="$CFLAGS -D__STRICT_ANSI__"
  CXXFLAGS="$CXXFLAGS -D__STRICT_ANSI__"
fi
AC_MSG_RESULT([$need_strict_ansi])

# Check if "std::shared_ptr" is "std::tr1::shared_ptr"
AC_MSG_CHECKING([for std::shared_ptr])
AC_LANG_PUSH(C++)
AC_TRY_COMPILE([
#include <memory>
],[
class A {};
std::shared_ptr<A> a;
],[has_std_shared_ptr=yes],[has_std_shared_ptr=no]);
AC_MSG_RESULT([$has_std_shared_ptr])
if eval "test x$has_std_shared_ptr = xno"; then
  AC_MSG_CHECKING([for std::tr1::shared_ptr])
  AC_TRY_COMPILE([
  #include <tr1/memory>
  ],[
  class A {};
  std::tr1::shared_ptr<A> a;
  ],[has_std_tr1_shared_ptr=yes],[has_std_tr1_shared_ptr=no]);
  AC_MSG_RESULT([$has_std_tr1_shared_ptr])
  if eval "test x$has_std_tr1_shared_ptr = xyes"; then
    AC_DEFINE(USE_STD_TR1_NAMESPACE,1,[Define to 1 if the std::tr1 namespace should be included in the std namespace.])
  fi
fi
AC_LANG_POP(C++)

# Check if "std::move" is available
AC_MSG_CHECKING([for std::move])
AC_LANG_PUSH(C++)
AC_TRY_COMPILE([
#include <utility>
],[
class A {};
A* a = new A();
A* b = std::move(a);
],[has_std_move=yes],[has_std_move=no]);
AC_LANG_POP(C++)
if eval "test x$has_std_move = xno"; then
  AC_DEFINE(NEED_STD_MOVE_FALLBACK,1,[Define to 1 if a fallback for "std::move" is required.])
fi
AC_MSG_RESULT([$has_std_move])

# Check if "nullptr" is available
AC_MSG_CHECKING([for nullptr])
AC_LANG_PUSH(C++)
AC_TRY_COMPILE([
],[
class A {};
A* a = nullptr;
],[has_nullptr=yes],[has_nullptr=no]);
AC_LANG_POP(C++)
if eval "test x$has_nullptr = xno"; then
  AC_DEFINE(NEED_NULLPTR_FALLBACK,1,[Define to 1 if a fallback for "nullptr" is required.])
fi
AC_MSG_RESULT([$has_nullptr])

# --- machine dependent optimizations ---

#AX_EXT

AC_ARG_ENABLE(sse,
              [AS_HELP_STRING([--disable-sse],
                              [disable SSE optimizations (default=no)])],
  [disable_sse=yes],
  [disable_sse=no])

if eval "test x$disable_sse != xyes"; then
    case $target_cpu in
      powerpc*)
        ;;

      i[[3456]]86*|x86_64*|amd64*)

        AX_CHECK_COMPILE_FLAG(-msse4.1, ax_cv_support_sse41_ext=yes, [])
        if test x"$ax_cv_support_sse41_ext" = x"yes"; then
#          SIMD_FLAGS="$SIMD_FLAGS -msse4.1"
          AC_DEFINE(HAVE_SSE4_1,1,[Support SSSE4.1 (Streaming SIMD Extensions 4.1) instructions])
        else
          AC_MSG_WARN([Your compiler does not support SSE4.1 instructions, can you try another compiler?])
        fi
        ;;

    esac
fi
AM_CONDITIONAL([ENABLE_SSE_OPT], [test x"$ax_cv_support_sse41_ext" = x"yes"])

# CFLAGS+=$SIMD_FLAGS
# CFLAGS+=" -march=x86-64"

case $target_cpu in
  arm*)
    AC_ARG_ENABLE(arm,
                  [AS_HELP_STRING([--disable-arm],
                                  [disable ARM optimizations (default=no)])],
      [disable_arm=yes],
      [disable_arm=no])

    if test x"$disable_arm" != x"yes"; then
      AC_DEFINE(HAVE_ARM, 1, [Support ARM instructions])

      AX_CHECK_COMPILE_FLAG(-mfpu=neon, [
          AC_DEFINE(HAVE_NEON, 1, [Support ARM NEON instructions])
          ax_cv_support_neon_ext=yes], [])

      AC_ARG_ENABLE(thumb,
                    [AS_HELP_STRING([--enable-thumb],
                                    [disable ARM THUMB instructions (default=no)])],
        [enable_thumb=yes],
        [enable_thumb=no])
    fi
    ;;

  *)
    disable_arm=yes
    ;;

esac

AM_CONDITIONAL([ENABLE_ARM_OPT], [test x"$disable_arm" != x"yes"])
AM_CONDITIONAL([ENABLE_NEON_OPT], [test x"$ax_cv_support_neon_ext" = x"yes"])
AM_CONDITIONAL([ENABLE_ARM_THUMB], [test x"$enable_thumb" != x"no"])

# --- additional logging ---

AC_ARG_ENABLE(log-error,
              [AS_HELP_STRING([--enable-log-error],
                              [turn on logging at error level (default=yes)])],
  [enable_log_error=$enableval],
  [enable_log_error=yes])
if eval "test $enable_log_error = yes"; then
  CXXFLAGS="$CXXFLAGS -DDE265_LOG_ERROR"
fi

AC_ARG_ENABLE(log-info,
              [AS_HELP_STRING([--enable-log-info],
                              [turn on logging at info level (default=no)])],
  [enable_log_info=$enableval],
  [enable_log_info=no])
if eval "test $enable_log_info = yes"; then
  CXXFLAGS="$CXXFLAGS -DDE265_LOG_INFO"
fi

AC_ARG_ENABLE(log-debug,
              [AS_HELP_STRING([--enable-log-debug],
                              [turn on logging at debug level (default=no)])],
  [enable_log_debug=$enableval],
  [enable_log_debug=no])
if eval "test $enable_log_debug = yes"; then
  CXXFLAGS="$CXXFLAGS -DDE265_LOG_DEBUG"
fi

AC_ARG_ENABLE(log-trace,
              [AS_HELP_STRING([--enable-log-trace],
                              [turn on logging at trace level (default=no)])],
  [enable_log_trace=$enableval],
  [enable_log_trace=no])
if eval "test $enable_log_trace = yes"; then
  CXXFLAGS="$CXXFLAGS -DDE265_LOG_TRACE"
fi


# --- enable example programs ---

AC_ARG_ENABLE([dec265], AS_HELP_STRING([--disable-dec265], [Do not build dec265 decoder program.]))
AC_ARG_ENABLE([sherlock265], AS_HELP_STRING([--disable-sherlock265], [Do not build sherlock265 visual inspection program.]))

if eval "test x$enable_dec265 = x"      ; then enable_dec265=yes ; fi
if eval "test x$enable_sherlock265 = x" ; then enable_sherlock265=yes ; fi

if eval "test x$enable_dec265 = xyes" || eval "test x$enable_sherlock265 = xyes" ; then
   PKG_CHECK_MODULES([VIDEOGFX], [libvideogfx],
                     [AC_DEFINE([HAVE_VIDEOGFX], [1], [Whether libvideogfx was found.])
                      AC_SUBST(VIDEOGFX_CFLAGS)
                      AC_SUBST(VIDEOGFX_LIBS)
                      have_videogfx="yes"],
                     [have_videogfx="no"]
                     )
fi

if eval "test x$enable_dec265 = xyes" ; then
   PKG_CHECK_MODULES([SDL], [sdl],
                     [AC_DEFINE([HAVE_SDL], [1], [Whether libsdl was found.])
                      AC_SUBST(SDL_CFLAGS)
                      AC_SUBST(SDL_LIBS)
                      have_sdl="yes"],
                     [have_sdl="no"]
                     )
fi

if eval "test x$enable_sherlock265 = xyes" && eval "test x$have_videogfx != xyes" ; then
   PKG_CHECK_MODULES([SWSCALE], [libswscale],
                     [AC_DEFINE([HAVE_SWSCALE], [1], [Whether libswscale was found.])
                      AC_SUBST(SWSCALE_CFLAGS)
                      AC_SUBST(SWSCALE_LIBS)
                      have_swscale="yes"],
                     [have_swscale="no"]
                     )
fi

AM_CONDITIONAL([HAVE_VIDEOGFX], [test "x$have_videogfx" = "xyes"])
AM_CONDITIONAL([HAVE_SWSCALE], [test "x$have_swscale" = "xyes"])
AM_CONDITIONAL([HAVE_SDL], [test "x$have_sdl" = "xyes"])

if eval "test $enable_dec265 = yes" && eval "test $have_videogfx != yes" && eval "test $have_sdl != yes" ; then
  AC_MSG_WARN([Did not find libvideogfx or libsdl, video output of dec265 will be disabled.])
fi

if eval "test $enable_sherlock265 = yes" && eval "test $have_videogfx != yes" && eval "test $have_swscale != yes" ; then
  AC_MSG_WARN([Did not find libvideogfx or libswscale, compilation of sherlock265 will be disabled.])
  enable_sherlock265="no"
fi

if eval "test $enable_sherlock265 = yes" ; then
  PKG_CHECK_MODULES([QT], [Qt5Core Qt5Gui Qt5Widgets], [found_qt=5], [found_qt=no])
  if eval "test $found_qt = no" ; then
    PKG_CHECK_MODULES([QT], [QtCore QtGui], [found_qt=4])
  fi
  AC_PATH_PROGS([QTCHOOSER],[qtchooser])
  if eval "test x$QTCHOOSER = x" ; then
    AC_PATH_PROGS([QTMOC],[moc-qt$found_qt moc])
    if eval "test x$QTMOC = x" ; then
      AC_MSG_ERROR([Need the "moc" commandline tool which is required to generate the Qt files required for sherlock265.])
    fi
  else
    QTMOC="$QTCHOOSER -run-tool=moc -qt=$found_qt"
  fi
  AC_MSG_CHECKING([for version of $QTMOC])
  QTMOC_VERSION=`$QTMOC -v 2>&1 | $GREP -o '[[0-9]]\+.[[0-9]]\+.[[0-9]]\+'`
  AC_MSG_RESULT([$QTMOC_VERSION])
  AX_COMPARE_VERSION([$QTMOC_VERSION], [lt], [$found_qt],
      [AC_MSG_ERROR([Please install "moc" for Qt$found_qt (found $QTMOC_VERSION).])])
  AC_SUBST(QTMOC)
fi

AM_CONDITIONAL([ENABLE_DEC265], [test "x$enable_dec265" != "xno"])
AM_CONDITIONAL([ENABLE_SHERLOCK265], [test "x$enable_sherlock265" != "xno"])


# --- output configuration results ---

AC_MSG_NOTICE([---------------------------------------])
AC_MSG_NOTICE([Building dec265 example: $enable_dec265])
AC_MSG_NOTICE([Building sherlock265 example: $enable_sherlock265])
AC_MSG_NOTICE([---------------------------------------])

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([libde265/Makefile])
AC_CONFIG_FILES([libde265/arm/Makefile])
AC_CONFIG_FILES([libde265/x86/Makefile])
AC_CONFIG_FILES([libde265/encoder/Makefile])
AC_CONFIG_FILES([libde265/encoder/algo/Makefile])
AC_CONFIG_FILES([libde265/de265-version.h])
AC_CONFIG_FILES([dec265/Makefile])
AC_CONFIG_FILES([enc265/Makefile])
AC_CONFIG_FILES([sherlock265/Makefile])
AC_CONFIG_FILES([tools/Makefile])
AC_CONFIG_FILES([acceleration-speed/Makefile])
AC_CONFIG_FILES([libde265.pc])
AC_OUTPUT

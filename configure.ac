#
# ------------------------------------------------------------------------------
# configure.ac
# ------------------------------------------------------------------------------
#
# Tyler Wayne © 2021
#

AC_PREREQ([2.69])
AC_INIT([Explorer], [0.1], [tylerwayne3@gmail.com])
AM_INIT_AUTOMAKE([subdir-objects])

LT_PREREQ([2.2.])
LT_INIT([dlopen])

AC_CONFIG_MACRO_DIRS([m4])
AC_CONFIG_SRCDIR([src/explorer.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_SEARCH_LIBS([dlopen], [dl])
AC_SEARCH_LIBS([initscr], [ncurses])
AC_SEARCH_LIBS([Mem_alloc], [mem])
AC_SEARCH_LIBS([assert], [error])
AC_SEARCH_LIBS([get_line], [cstrings])

# Checks for header files.
AC_CHECK_HEADERS([])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([])

AC_CONFIG_FILES([
  Makefile
  common/Makefile
  include/Makefile
  src/Makefile
  test/Makefile])

AC_OUTPUT

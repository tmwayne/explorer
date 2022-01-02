#
# ------------------------------------------------------------------------------
# configure.ac
# ------------------------------------------------------------------------------
#
# Tyler Wayne © 2021
#

AC_PREREQ([2.69])
AC_INIT([Preview], [0.1], [tylerwayne3@gmail.com])
AM_INIT_AUTOMAKE([subdir-objects])

LT_PREREQ([2.2.])
LT_INIT([dlopen])

AC_CONFIG_MACRO_DIRS([m4])
AC_CONFIG_SRCDIR([src/preview/preview.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_YACC
AC_PROG_LEX([noyywrap])

# Checks for libraries.
AC_SEARCH_LIBS([dlopen], [dl])
AC_SEARCH_LIBS([initscr], [ncurses])

# Checks for header files.
AC_CHECK_HEADERS([])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([])

AC_CONFIG_FILES([
  Makefile
  include/Makefile
  src/Makefile
  src/common/Makefile
  src/input-parser/Makefile
  src/preview/Makefile
  test/Makefile
  test/data/Makefile
  test/unit/Makefile
  test/integration/Makefile])

AC_OUTPUT

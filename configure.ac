#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([alsep_tools],[0.5],[yukio@planeta.sci.isas.jaxa.jp])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_HEADERS([config.h])
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([limits.h stdint.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_STRFTIME
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([memset])

AC_CONFIG_FILES([Makefile
                 info/Makefile
                 lib/Makefile
                 pgcopy/Makefile
                 script/Makefile
                 csv/Makefile
                 sql/Makefile
                 csv_for_d5a/Makefile])
AC_PROG_RANLIB
AC_OUTPUT

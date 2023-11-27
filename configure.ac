#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.71])
AC_INIT([BistaticHub], [2.0], [BUG-REPORT-ADDRESS])
AM_INIT_AUTOMAKE([foreign -Wall])
AC_CONFIG_SRCDIR([BistaticHub.cc])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([netcdf], [nc_open])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netinet/in.h sys/socket.h sys/time.h sys/vfs.h unistd.h values.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_SIZE_T
AC_CHECK_HEADER_STDBOOL

# Checks for library functions.
AC_FUNC_STRTOD
AC_CHECK_FUNCS([alarm gettimeofday inet_ntoa memchr memmove memset mkdir pow select socket sqrt strchr strrchr])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

# Process this file with autoconf to produce a configure script.
AC_INIT(BistaticHub, 2.0.0, rdp-support@atd.ucar.edu)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([Beam.cc])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([netcdf], [nc_open])

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netinet/in.h stdlib.h string.h sys/socket.h sys/time.h sys/vfs.h unistd.h values.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_SETVBUF_REVERSED
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_FUNC_STRTOD
AC_CHECK_FUNCS([alarm gettimeofday inet_ntoa memchr memmove memset mkdir pow select socket sqrt strchr strrchr])

AC_OUTPUT([Makefile])

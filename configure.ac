#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([twilc], [0.1], [pipituliuliu@gmail.com])
AC_CONFIG_SRCDIR([src/main.c])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE()

# Checks for programs.
AC_PROG_CC

AC_CHECK_LIB([m], [cos], ,AC_ERROR(libm not found))
AC_CHECK_LIB([ncursesw], [initscr], ,AC_ERROR(ncursesw not found))
AC_CHECK_LIB([oauth], [oauth_sign_url2], ,AC_ERROR(liboauth not found))
AC_CHECK_LIB([z], [gzread], ,AC_ERROR(zlib not found))
AC_CHECK_LIB([curl],[curl_global_init], ,AC_ERROR(libcurl not found))
AC_CHECK_LIB([pthread],[pthread_mutex_init], , AC_ERROR(pthread not found))
AC_CHECK_LIB([jansson],[json_loads], , AC_ERROR(jansson not found))

AC_PROG_CC_C99
if test "x$ac_cv_prog_cc_c99" = xno; then
    AC_MSG_ERROR([You need a compiler that supports c99])
fi

# Checks for header files.
AC_CHECK_HEADERS([locale.h stdint.h stdlib.h string.h])

# We need c99
CFLAGS="$CFLAGS -std=gnu99 -I/usr/include/glib-2.0 -I/usr/local/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/local/lib/glib-2.0/include"

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset setlocale strdup strstr])

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT

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
AC_CHECK_LIB([xml2], [xmlDocGetRootElement], ,AC_ERROR(libxml2 not found))
AC_CHECK_LIB([z], [gzread], ,AC_ERROR(zlib not found))
AC_CHECK_LIB([curl],[curl_global_init], ,AC_ERROR(libcurl not found))
AC_CHECK_LIB([pthread],[pthread_mutex_init], , AC_ERROR(pthread not found))

# Checks for header files.
AC_CHECK_HEADERS([locale.h stdint.h stdlib.h string.h])

# Determine XML2 include path
AC_MSG_CHECKING(for libxml/xmlmemory.h)
# Can we include headers using system include dirs?
AC_TRY_COMPILE([#include <libxml/xmlmemory.h>], [int a = 1;],
    XML2_INCLUDE=" ",
    XML2_INCLUDE=
)
# Hunt through several possible directories to find the includes for libxml2
if test "x$XML2_INCLUDE" = "x"; then
    old_LDFLAGS="$LDFLAGS"
    for i in $xml2_include_dir /usr/include /usr/local/include /usr/include/libxml2 /usr/local/include/libxml2 ; do
        CPPFLAGS="$old_LDFLAGS -I$i"
        AC_TRY_COMPILE([#include <libxml/xmlmemory.h>], [int a = 1;],
            XML2_INCLUDE="-I$i",
            XML2_INCLUDE=
        )
        if test "x$XML2_INCLUDE" != "x"; then
            break;
        fi
    done
    LDFLAGS="$old_LDFLAGS $XML2_INCLUDE"
fi
# We need c99
CFLAGS="$CFLAGS -std=gnu99"

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset setlocale strdup strstr])

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT

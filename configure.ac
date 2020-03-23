#                                               -*- Autoconf -*-


#  Copyright (c) 2017, 2020  Dale Mellor
#
#   This file is part of the trader-desk package.
#
#   The trader-desk package is free software: you can redistribute it
#   and/or modify it under the terms of the GNU General Public License as
#   published by the Free Software Foundation, either version 3 of the
#   License, or (at your option) any later version.
#
#   The trader-desk package is distributed in the hope that it will be
#   useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
#   General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program. If not, see http://www.gnu.org/licenses/.


# Process this file with autoconf to produce a configure script.


AC_PREREQ(2.69)
AC_INIT([trader-desk], [0.1], [dale@rdmp.org])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([silent-rules subdir-objects])
AM_SILENT_RULES([yes])
AC_CONFIG_SRCDIR([trader-desk/trader-desk.cc])
AC_CONFIG_HEADERS([trader-desk/auto-config.h])
AC_CONFIG_MACRO_DIRS([m4])

# Checks for programs.
AC_PROG_CC
AC_GNU_SOURCE
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.19.3])

AC_CHECK_PROGS([mysql_config], [mariadb_config mysql_config], [no])
if [[ "x$mysql_config" == "xno" ]]; then
   AC_MSG_FAILURE([cannot find MySQL-type client libraries])
fi
if [[ "x$mysql_config" == "xmysql_config" ]]; then
   AC_SUBST([HAVE_MYSQL], [1])
else
   AC_SUBST([HAVE_MYSQL], [0])
fi
if [[ "x$mysql_config" == "xmariadb_config" ]]; then
   AC_SUBST([HAVE_MARIADB], [1])
else
   AC_SUBST([HAVE_MARIADB], [0])
fi

# Checks for libraries.
PKG_CHECK_MODULES([gtk_config], [gtkmm-3.0 gthread-2.0 dmbcs-market-data-api fmt])
gtk_config_CFLAGS="${pkg_cv_gtk_config_CFLAGS} `${mysql_config} --include`"
gtk_config_LIBS="${pkg_cv_gtk_config_LIBS} `${mysql_config} --libs`"

# Checks for header files.
AC_CHECK_HEADERS([libintl.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T

# Checks for library functions.
AC_FUNC_MKTIME
AC_FUNC_STRTOD
AC_CHECK_FUNCS([floor localtime_r pow setlocale sqrt])

# Check for optional database pre-population data.
AM_CONDITIONAL([database_data], [test -f data.sql.xz])


AC_CONFIG_FILES([trader-desk.pc
                 makefile
                 po/Makefile.in
                 trader-desk/makefile])
dnl                 doc/makefile

AC_OUTPUT

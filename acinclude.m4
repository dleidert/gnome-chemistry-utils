dnl Copying and distribution of this file, with or without modification,
dnl are permitted in any medium without royalty provided the copyright
dnl notice and this notice are preserved.


dnl @synopsis GCU_PROG_XMLLINT
dnl
dnl @summary Determine if we can use the xmllint program
dnl
dnl This is a simple macro to define the location of xmllint (which can
dnl be overridden by the user) and special options to use.
dnl
dnl @category InstalledPackages
dnl @author Daniel Leidert <daniel.leidert@wgdd.de>
dnl @version 2006-09-24
dnl @license AllPermissive
AC_DEFUN([GCU_PROG_XMLLINT],[
AC_ARG_VAR(
	[XMLLINT],
	[The `xmllint' binary with path. Use it to define or override the location of `xmllint'.]
)
AC_PATH_PROG([XMLLINT], [xmllint])
if test -z $XMLLINT ; then
	AC_MSG_WARN([`xmllint' was not found. We cannot validate the XML sources.]) ;
else
	AC_MSG_CHECKING([for xmllint >= 2.6.24...])
	m4_ifdef(
		[PKG_CHECK_EXISTS],
		[
		 PKG_CHECK_EXISTS(
		 	[libxml-2.0 >= 2.6.24],
		 	[AC_MSG_RESULT([yes])],
		 	[
		 	 AC_MSG_RESULT([no])
		 	 XMLLINT=""
		 	 AC_MSG_WARN([`xmllint' not available or too old. We cannot validate the XML sources.])
		 	]
		 )
		],
		[
		 if $PKG_CONFIG libxml-2.0 --atleast-version=2.6.24; then
		 	AC_MSG_RESULT([yes])
		 else
		 	AC_MSG_RESULT([no])
		 	XMLLINT=""
		 	AC_MSG_WARN([`xmllint' not available or too old. We cannot validate the XML sources.])
		 fi
		]
	)
fi
AC_SUBST([XMLLINT])
AM_CONDITIONAL([HAVE_XMLLINT], [test "x$XMLLINT" != "x"])

AC_ARG_VAR(
	[XMLLINT_FLAGS],
	[More options, which should be used along with `xmllint', like e.g. `--nonet'.]
)
AC_SUBST([XMLLINT_FLAGS])
AC_MSG_CHECKING([for optional xmllint options to use...])
AC_MSG_RESULT([$XMLLINT_FLAGS])
]) # GCU_PROG_XMLLINT


dnl @synopsis GCU_PROG_XSLTPROC
dnl
dnl @summary Determine if we can use the xsltproc program
dnl
dnl This is a simple macro to define the location of xsltproc (which can
dnl be overridden by the user) and special options to use.
dnl
dnl @category InstalledPackages
dnl @author Daniel Leidert <daniel.leidert@wgdd.de>
dnl @version 2006-009-24
dnl @license AllPermissive
AC_DEFUN([GCU_PROG_XSLTPROC],[
AC_ARG_VAR(
	[XSLTPROC],
	[The `xsltproc' binary with path. Use it to define or override the location of `xsltproc'.]
)
AC_PATH_PROG([XSLTPROC], [xsltproc])
if test -z $XSLTPROC ; then
	AC_MSG_WARN([`xsltproc' was not found! The manpages cannot be updated from their XML source.]) ;
fi
AC_SUBST([XSLTPROC])
AM_CONDITIONAL([HAVE_XSLTPROC], [test "x$XSLTPROC" != "x"])

AC_ARG_VAR(
	[XSLTPROC_FLAGS],
	[More options, which should be used along with `xsltproc', like e.g. `--nonet'.]
)
AC_SUBST([XSLTPROC_FLAGS])
AC_MSG_CHECKING([for optional xsltproc options to use...])
AC_MSG_RESULT([$XSLTPROC_FLAGS])
]) # GCU_PROG_XSLTPROC


dnl @synopsis GCU_PROG_MAN
dnl
dnl @summary Determine if we can use the man program
dnl
dnl This is a simple macro to define the location of man (which can
dnl be overridden by the user).
dnl
dnl @category InstalledPackages
dnl @author Daniel Leidert <daniel.leidert@wgdd.de>
dnl @version 2006-09-24
dnl @license AllPermissive
AC_DEFUN([GCU_PROG_MAN],[
AC_ARG_VAR(
	[MAN],
	[The `man' binary with path. Use it to define or override the location of `man'.]
)
AC_PATH_PROG([MAN], [man])
if test -z $MAN ; then
	AC_MSG_WARN([`man' was not found. We cannot check the manpages for errors. See README.]) ;
fi
AC_SUBST([MAN])
AM_CONDITIONAL([HAVE_MAN], [test "x$MAN" != "x"])
]) # GCU_PROG_MAN

dnl Copying and distribution of this file, with or without modification,
dnl are permitted in any medium without royalty provided the copyright
dnl notice and this notice are preserved.


dnl @synopsis MP_PROG_XMLLINT
dnl
dnl @summary Determine if we can use the xmllint program
dnl
dnl This is a simple macro to define the location of xmllint (which can
dnl be overridden by the user) and special options to use.
dnl
dnl @category InstalledPackages
dnl @author Daniel Leidert <daniel.leidert@wgdd.de>
dnl @version 2006-05-06
dnl @license AllPermissive
AC_DEFUN([MP_PROG_XMLLINT],[
AC_ARG_VAR(
	[XMLLINT],
	[The 'xmllint' binary with path. Use it to define or override the location of 'xmllint'.]
)
AC_PATH_PROG([XMLLINT], [xmllint])
if test -z $XMLLINT ; then
	AC_MSG_WARN(['xmllint' was not found. We cannot validate the XML sources.]) ;
else
	echo -n "checking for xmllint >= 2.6.24... "
	PKG_CHECK_EXISTS(
		[libxml-2.0 >= 2.6.24],
		[echo "yes"],
		[echo "no"  ; XMLLINT="" ; AC_MSG_WARN(['xmllint' too old. We cannot validate the XML sources.])]
	)
fi
AC_SUBST([XMLLINT])
AM_CONDITIONAL([HAVE_XMLLINT], [test "x$XMLLINT" != "x"])

AC_ARG_VAR(
	[XMLLINT_FLAGS],
	[More options, which should be used along with 'xmllint', like e.g. '--nonet'.]
)
AC_SUBST([XMLLINT_FLAGS])
echo -n "checking for optional xmllint options to use... "
echo $XMLLINT_FLAGS
]) # MP_PROG_XMLLINT


dnl @synopsis MP_PROG_XSLTPROC
dnl
dnl @summary Determine if we can use the xsltproc program
dnl
dnl This is a simple macro to define the location of xsltproc (which can
dnl be overridden by the user) and special options to use.
dnl
dnl @category InstalledPackages
dnl @author Daniel Leidert <daniel.leidert@wgdd.de>
dnl @version 2006-03-10
dnl @license AllPermissive
AC_DEFUN([MP_PROG_XSLTPROC],[
AC_ARG_VAR(
	[XSLTPROC],
	[The 'xsltproc' binary with path. Use it to define or override the location of 'xsltproc'.]
)
AC_PATH_PROG([XSLTPROC], [xsltproc])
if test -z $XSLTPROC ; then
	AC_MSG_ERROR(['xsltproc' was not found! We cannot proceed. See README.]) ;
fi
AC_SUBST([XSLTPROC])

AC_ARG_VAR(
	[XSLTPROC_FLAGS],
	[More options, which should be used along with 'xsltproc', like e.g. '--nonet'.]
)
AC_SUBST([XSLTPROC_FLAGS])
echo -n "checking for optional xsltproc options to use... "
echo $XSLTPROC_FLAGS
]) # MP_PROG_XSLTPROC


dnl @synopsis MP_PROG_MAN
dnl
dnl @summary Determine if we can use the man program
dnl
dnl This is a simple macro to define the location of man (which can
dnl be overridden by the user).
dnl
dnl @category InstalledPackages
dnl @author Daniel Leidert <daniel.leidert@wgdd.de>
dnl @version 2006-03-10
dnl @license AllPermissive
AC_DEFUN([MP_PROG_MAN],[
AC_ARG_VAR(
	[MAN],
	[The 'man' binary with path. Use it to define or override the location of 'man'.]
)
AC_PATH_PROG([MAN], [man])
if test -z $MAN ; then
	AC_MSG_WARN(['man' was not found. We cannot check the manpages for errors. See README.]) ;
fi
AC_SUBST([MAN])
AM_CONDITIONAL([HAVE_MAN], [test "x$MAN" != "x"])
]) # MP_PROG_MAN

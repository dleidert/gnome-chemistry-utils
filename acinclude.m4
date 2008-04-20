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

dnl dolt, a replacement for libtool
dnl Copyright © 2007-2008 Josh Triplett <josh@freedesktop.org>
dnl Copying and distribution of this file, with or without modification,
dnl are permitted in any medium without royalty provided the copyright
dnl notice and this notice are preserved.
dnl
dnl To use dolt, invoke the DOLT macro immediately after the libtool macros.
dnl Optionally, copy this file into acinclude.m4, to avoid the need to have it
dnl installed when running autoconf on your project.

AC_DEFUN([DOLT], [
AC_REQUIRE([AC_CANONICAL_HOST])
# dolt, a replacement for libtool
# Josh Triplett <josh@freedesktop.org>
AC_PATH_PROG(DOLT_BASH, bash)
AC_MSG_CHECKING([if dolt supports this host])
dolt_supported=yes
if test x$DOLT_BASH = x; then
    dolt_supported=no
fi
if test x$GCC != xyes; then
    dolt_supported=no
fi
case $host in
i?86-*-linux*|x86_64-*-linux*|powerpc-*-linux*) ;;
amd64-*-freebsd*|i?86-*-freebsd*|ia64-*-freebsd*) ;;
*) dolt_supported=no ;;
esac
if test x$dolt_supported = xno ; then
    AC_MSG_RESULT([no, falling back to libtool])
    LTCOMPILE='$(LIBTOOL) --tag=CC $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=compile $(COMPILE)'
    LTCXXCOMPILE='$(LIBTOOL) --tag=CXX $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=compile $(CXXCOMPILE)'
else
    AC_MSG_RESULT([yes, replacing libtool])

dnl Start writing out doltcompile.
    cat <<__DOLTCOMPILE__EOF__ >doltcompile
#!$DOLT_BASH
__DOLTCOMPILE__EOF__
    cat <<'__DOLTCOMPILE__EOF__' >>doltcompile
args=("$[]@")
for ((arg=0; arg<${#args@<:@@@:>@}; arg++)) ; do
    if test x"${args@<:@$arg@:>@}" = x-o ; then
        objarg=$((arg+1))
        break
    fi
done
if test x$objarg = x ; then
    echo 'Error: no -o on compiler command line' 1>&2
    exit 1
fi
lo="${args@<:@$objarg@:>@}"
obj="${lo%.lo}"
if test x"$lo" = x"$obj" ; then
    echo "Error: libtool object file name \"$lo\" does not end in .lo" 1>&2
    exit 1
fi
objbase="${obj##*/}"
__DOLTCOMPILE__EOF__

dnl Write out shared compilation code.
    if test x$enable_shared = xyes; then
        cat <<'__DOLTCOMPILE__EOF__' >>doltcompile
libobjdir="${obj%$objbase}.libs"
if test ! -d "$libobjdir" ; then
    mkdir_out="$(mkdir "$libobjdir" 2>&1)"
    mkdir_ret=$?
    if test "$mkdir_ret" -ne 0 && test ! -d "$libobjdir" ; then
	echo "$mkdir_out" 1>&2
        exit $mkdir_ret
    fi
fi
pic_object="$libobjdir/$objbase.o"
args@<:@$objarg@:>@="$pic_object"
"${args@<:@@@:>@}" -fPIC -DPIC || exit $?
__DOLTCOMPILE__EOF__
    fi

dnl Write out static compilation code.
dnl Avoid duplicate compiler output if also building shared objects.
    if test x$enable_static = xyes; then
        cat <<'__DOLTCOMPILE__EOF__' >>doltcompile
non_pic_object="$obj.o"
args@<:@$objarg@:>@="$non_pic_object"
__DOLTCOMPILE__EOF__
        if test x$enable_shared = xyes; then
            cat <<'__DOLTCOMPILE__EOF__' >>doltcompile
"${args@<:@@@:>@}" >/dev/null 2>&1 || exit $?
__DOLTCOMPILE__EOF__
        else
            cat <<'__DOLTCOMPILE__EOF__' >>doltcompile
"${args@<:@@@:>@}" || exit $?
__DOLTCOMPILE__EOF__
        fi
    fi

dnl Write out the code to write the .lo file.
dnl The second line of the .lo file must match "^# Generated by .*libtool"
    cat <<'__DOLTCOMPILE__EOF__' >>doltcompile
{
echo "# $lo - a libtool object file"
echo "# Generated by doltcompile, not libtool"
__DOLTCOMPILE__EOF__

    if test x$enable_shared = xyes; then
        cat <<'__DOLTCOMPILE__EOF__' >>doltcompile
echo "pic_object='$pic_object'"
__DOLTCOMPILE__EOF__
    else
        cat <<'__DOLTCOMPILE__EOF__' >>doltcompile
echo pic_object=none
__DOLTCOMPILE__EOF__
    fi

    if test x$enable_static = xyes; then
        cat <<'__DOLTCOMPILE__EOF__' >>doltcompile
echo "non_pic_object='$non_pic_object'"
__DOLTCOMPILE__EOF__
    else
        cat <<'__DOLTCOMPILE__EOF__' >>doltcompile
echo non_pic_object=none
__DOLTCOMPILE__EOF__
    fi

    cat <<'__DOLTCOMPILE__EOF__' >>doltcompile
} > "$lo"
__DOLTCOMPILE__EOF__

dnl Done writing out doltcompile; substitute it for libtool compilation.
    chmod +x doltcompile
    LTCOMPILE='$(top_builddir)/doltcompile $(COMPILE)'
    LTCXXCOMPILE='$(top_builddir)/doltcompile $(CXXCOMPILE)'
fi
AC_SUBST(LTCOMPILE)
AC_SUBST(LTCXXCOMPILE)
# end dolt
])

#!/bin/sh

## check for doxygen present. Things don't work if it is not there
if [ -z `doxygen --version` ]; then
	echo "You need doxygen to build this package from cvs."
	echo "http://www.doxygen.org/"
	exit 1
fi

## First, find where automake is installed and get the version
AMPATH=`which automake|sed 's/\/bin\/automake//'`
AMVER=`automake --version|grep automake|awk '{print $4}'|awk -F. '{print $1"."$2}'`

## Copy files from the automake directory
ln -sf $AMPATH/share/automake-$AMVER/missing .
ln -sf $AMPATH/share/automake-$AMVER/mkinstalldirs .
ln -sf $AMPATH/share/automake-$AMVER/install-sh .
ln -sf $AMPATH/share/automake-$AMVER/depcomp .
ln -sf $AMPATH/share/automake-$AMVER/compile .

## run aclocal
if [ -r /usr/share/aclocal/gnome-common.m4 ]; then
	aclocal
else
	aclocal -I /usr/share/aclocal/gnome2-macros
fi

## libtool and intltool
libtoolize --force
intltoolize --force
gnome-doc-prepare --force

## autoheader, automake, autoconf
autoheader
automake --gnu -af
autoconf

## Job ended
echo "run ./configure with the appropriate options, then make and enjoy"

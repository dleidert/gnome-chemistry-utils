#!/bin/sh

## First, find where automake is installed and get the version
AMPATH=`which automake|sed 's/\/bin\/automake//'`
AMVER=`automake --version|grep automake|awk '{print $4}'|awk -F. '{print $1"."$2}'`

## Copy files from the automake directory
ln -sf $AMPATH/share/automake-$AMVER/missing .
ln -sf $AMPATH/share/automake-$AMVER/mkinstalldirs .
ln -sf $AMPATH/share/automake-$AMVER/install-sh .
ln -sf $AMPATH/share/automake-$AMVER/depcomp .

## run aclocal
if [ -r /usr/share/aclocal/gnome-common.m4 ]; then
	aclocal
else
	aclocal -I /usr/share/aclocal/gnome2-macros
fi

## libtool and intltool
libtoolize --force
intltoolize --force

## generate documentation archive
cd docs/reference
doxygen Doxyfile
tar -czf html-upstream.tar.gz html
cd ../../

## autoheader, automake, autoconf
autoheader
automake --gnu
autoconf

## Job ended
echo "run ./configure with the appropriate options, the make and enjoy"

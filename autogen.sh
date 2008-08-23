#!/bin/bash

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

## generate versioned help files
GCU_VERSION=`grep AC_INIT configure.ac|gawk -F',' '{ print $2 }'|sed "s/.*\[//"|sed "s/\].*//"`
GCU_MAJOR_VERSION=`echo $GCU_VERSION | awk -F . '{ print $1}'`
GCU_MINOR_VERSION=`echo $GCU_VERSION | awk -F . '{ print $2}'`
let GCU_API_MINOR_VERSION=($GCU_MINOR_VERSION+1)/2*2
GCU_API_VER="$GCU_MAJOR_VERSION.$GCU_API_MINOR_VERSION"
for i in docs/help/*; do
	if [ -d $i ]; then
		product=`grep DOC_MODULE $i/Makefile.am|sed "s/.*=//"|sed "s/-.*//"|sed "s/ //"`
		cp "$i/$product.omf.in" "$i/$product-$GCU_API_VER.omf.in"
		for j in $i/*; do
			if [ -d $j ]; then
				cp "$j/$product.xml" "$j/$product-$GCU_API_VER.xml"
			fi
		done
	fi
done

## Job ended
echo "run ./configure with the appropriate options, then make and enjoy"

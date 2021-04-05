#!/bin/sh

CXX="g++" ; export CXX
CC="gcc" ; export CC
FC="gfortran"; export FC
F77="gfortran"; export F77

pushd mplapack/debug ; bash gen.Makefile.am.sh ; popd

aclocal ; autoconf ; automake --add-missing
autoreconf --force --install
if [ `uname -m` = "x86_64" ]; then
./configure --prefix=$HOME/MPLAPACK --enable-gmp=yes --enable-mpfr=yes --enable-_Float128=yes --enable-qd=yes --enable-dd=yes --enable-double=yes --enable-_Float64x=yes --enable-debug=yes
else
./configure --prefix=$HOME/MPLAPACK --enable-gmp=yes --enable-mpfr=yes --enable-_Float128=yes --enable-qd=yes --enable-dd=yes --enable-double=yes --enable-debug=yes
fi

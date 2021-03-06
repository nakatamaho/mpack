#!/bin/sh
#test on Linux
#ssh -N -L2002:tesla:22 maho@sdpa.indsys.chuo-u.ac.jp &
#add following lines to /home/maho/.ssh/config
#Host tesla
#Hostname localhost
#User maho
#Port 2002

MPLAPACK_VER=0.6.7
HOST=tesla
WORKDIR=mplapack-work-tesla
MPLAPACK_CONFIG_SH=configlinux.sh

MPLAPACK_DIST_DIR=mplapack-$MPLAPACK_VER
MPLAPACK_DIST=mplapack-$MPLAPACK_VER.tar.gz

#install mplapack
ssh $HOST "rm -rf $WORKDIR"
ssh $HOST "mkdir $WORKDIR"
scp $MPLAPACK_DIST $HOST:$WORKDIR
scp misc/$MPLAPACK_CONFIG_SH $HOST:$WORKDIR
ssh $HOST "cd $WORKDIR ; md5sum $MPLAPACK_DIST ; tar xvfz $MPLAPACK_DIST ; cd $MPLAPACK_DIST_DIR ; (sh ../$MPLAPACK_CONFIG_SH ; make -j4 ; make install) 2>&1 | tee log.linux" | tee log.linux_tesla

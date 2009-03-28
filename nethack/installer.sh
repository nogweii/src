#!/bin/sh

# A really simple installer, please configure it to your needs!
INSTALL_ROOT=$HOME/sys

if [ ! -f nethack-343-src.tgz ] ; then
	wget http://downloads.sourceforge.net/sourceforge/nethack/nethack-343-src.tgz
fi
if [ ! -d nethack-3.4.3 ] ; then
	tar xf nethack-343-src.tgz
fi
cd nethack-3.4.3

# Make sure the following patches are in this order, only way I know they work!
# 
# Of course, they could still work in different order, but I don't care to find
# all the possible ways they could.
#patch -Np1 -i "../nh343jl-combined-full.diff"
#patch -u -Np0 -i "../cflags.patch"
patch -Nsp1 -i "../patches/tricks.diff"
patch -Nsp1 -i "../patches/64bit.diff"
patch -Nsp1 -i "../patches/cookie.diff"
patch -Nsp1 -i "../patches/grudge.diff"
patch -Nsp1 -i "../patches/statuscolors.patch"
patch -Nsp1 -i "../patches/invis-objects.diff"
patch -Nsp1 -i "../patches/darkroom_glyph.diff"
patch -Nsp1 -i "../patches/use_darkgray.diff"
patch -Nsup1 -F3 -i "../patches/configurable_botl.diff"
patch -Nsup1 -F3 -i "../patches/extended_stats.diff"
# This is a modified patch to work with the above patches!
# For the original, check old_patches
patch -Nsp1 -i "../patches/menucolor.patch"
patch -Nsup1 -F3 -i "../patches/dungeoncolors.diff"
patch -Nsp1 -i "../patches/setmetamode.patch"
patch -Nsp1 -i "../patches/glyphcolor.diff"
patch -Nsp1 -i "../patches/toonhits.diff"
patch -Nsp1 -i "../patches/itemcat.diff"
patch -Nsp1 -i "../patches/quivfir.diff" 
patch -Nsp1 -i "../patches/fliplevel.diff"
patch -Nsp1 -i "../patches/priestrace.diff"
patch -Nsp1 -i "../patches/yafm.patch"
# Eh, bone file incompatible:
#patch -Nup1 -F3 -i "../patches/conducts.diff"
#patch -Nup1 -i "../patches/conducts_bone_compat.diff"
patch -Nsp0 -d src/ -i "../../patches/cat_tin.diff"
patch -Nsup1 -F3 -i "../patches/flipcoin.diff"
patch -Nsp1 -F3 -i "../patches/qm_erinys_names.diff"
# Modified version:
patch -Nsp1 -i "../patches/bonesdir.patch"
patch -Nsp1 -i "../patches/confused_whistle.diff"
patch -Nsp1 -i "../patches/forgetquit.diff"
patch -Nsp1 -F3 -i "../patches/logmoves.diff"
patch -Nsp1 -F3 -i "../patches/winedge.diff"
patch -Nsp1 -i "../patches/ownedarti.diff"
patch -Nsp1 -F3 -i "../patches/paranoid.diff"
patch -Nsp1 -F3 -i "../patches/xlog.patch"
# -F3-ing something caused pray.c & win/tty/termcap.c to have errors. This fixes it.
patch -Nsup1 -i "../patches/pray_paren.patch"
patch -Nsup1 -i "../patches/tparm_fix.patch"
# Don't use -ltermlib but -lncurses instead, among other changes
patch -Nup1 -i "../patches/unix-makefile-conf.patch"

# Clean up from the patching
find . -name "*.orig" -delete

# Let's make this bad boy!
cd sys/unix
sh setup.sh
cd ../..
#make
#exit 0

make PREFIX=$INSTALL_ROOT GAMEUID=$(id -rnu) GAMEGRP=$(id -rng)
make PREFIX=$INSTALL_ROOT GAMEUID=$(id -rnu) GAMEGRP=$(id -rng) install
install -Dm644 dat/license $INSTALL_ROOT/share/licenses/$pkgname/license

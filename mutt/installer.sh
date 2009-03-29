#!/bin/sh

# A really simple installer, please configure it to your needs!
INSTALL_ROOT=$HOME/sys
startdir=$(pwd)

if [ ! -f mutt-1.5.19.tar.gz ] ; then
	wget ftp://ftp.mutt.org/mutt/devel/mutt-1.5.19.tar.gz
fi
if [ ! -d mutt-1.5.19 ] ; then
	tar xf mutt-1.5.19.tar.gz
fi
cd mutt-1.5.19

patch -p1 -i "${startdir}/patches/mutt-1.5.19-imap_hook_fix-1.patch"
#patch -p1 -i "${startdir}/patch-1.5.19.rr.compressed.1"
patch -p1 -i "${startdir}/patches/mutt-sidebar-1.5.19.patch"
patch -p1 -F3 -i "${startdir}/patches/echo_command.patch"

./configure --prefix=/usr --sysconfdir=/etc \
	--enable-pop --enable-imap --enable-smtp \
	--with-sasl --with-ssl=/usr --without-idn \
	--enable-hcache --enable-pgp --enable-inodesort \
	--enable-compressed --with-regex \
	--enable-gpgme --with-slang=/usr

make PREFIX=$INSTALL_ROOT
make install
#rm -f ${startdir}/pkg/usr/bin/{flea,muttbug}
#rm -f $startdir/pkg/usr/share/man/man1/{flea,muttbug}.1
#rm -f ${startdir}/pkg/etc/mime.types*
install -Dm644 contrib/gpg.rc $INSTALL_ROOT/etc/Muttrc.gpg.dist

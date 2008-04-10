#!/bin/sh

export CAIRO_DOCK_DIR=/opt/cairo-dock/trunk
export FAST_COMPIL="0" 
export BUILD_TAR="0" 
if test "$1" = "-f" -o "$2" = "-f"; then
	export FAST_COMPIL="1"
fi
if test "$1" = "-t" -o "$2" = "-t"; then
	export BUILD_TAR="1"
fi

#\_____________ On nettoie tout et on fait une archive des sources.
cd $CAIRO_DOCK_DIR
find . -name "*~" -delete
find . -name "core*" -delete
find . -name ".#*" -delete
rm -f cairo-dock-sources*.tar.bz2 *.deb

cd $CAIRO_DOCK_DIR/themes
./cairo-dock-finalize-theme.sh

cd $CAIRO_DOCK_DIR/cairo-dock
./compile-all.sh -ct

cd $CAIRO_DOCK_DIR
sudo rm -rf deb/usr
sudo rm -rf deb-plug-ins/usr

cd $CAIRO_DOCK_DIR/deb
if test -e debian; then
	mv debian DEBIAN
fi
cd $CAIRO_DOCK_DIR/deb-plug-ins
if test -e debian; then
	mv debian DEBIAN
fi

if test "$BUILD_TAR" = "1"; then
	cd /tmp
	echo "* building tarball ..."
	tar cf `date +"cairo-dock-sources-%Y%m%d.tar"` $CAIRO_DOCK_DIR/cairo-dock $CAIRO_DOCK_DIR/plug-ins $CAIRO_DOCK_DIR/themes > /dev/null
	if test ! "$?" = "0"; then
		echo "  Attention : an error has occured !"
		echo "Error while building tarball" >> $CAIRO_DOCK_DIR/compile.log
	else
		echo "  -> passed"
	fi
	
	cd $CAIRO_DOCK_DIR
	mv `date +"/tmp/cairo-dock-sources-%Y%m%d.tar"` .
	bzip2 *.tar
fi


#\_____________ On compile de zero.
cd $CAIRO_DOCK_DIR/cairo-dock
if test "$FAST_COMPIL" = "1"; then
	./compile-all.sh -C -i -t
else
	./compile-all.sh -a -C -i -t
fi

#\_____________ On cree les paquets.
cd $CAIRO_DOCK_DIR/deb
sudo mkdir usr
sudo mkdir usr/bin
sudo mkdir usr/share
sudo mkdir usr/share/menu
sudo mkdir usr/share/applications
sudo mkdir usr/share/pixmaps
for lang in `cat ../cairo-dock/po/LINGUAS`; do
	sudo mkdir -p usr/share/locale/$lang/LC_MESSAGES
	sudo cp /usr/share/locale/$lang/LC_MESSAGES/cairo-dock.mo usr/share/locale/$lang/LC_MESSAGES
done;

sudo cp /usr/bin/cairo-dock usr/bin
sudo cp /usr/bin/launch-cairo-dock-after-beryl.sh usr/bin
sudo cp /usr/bin/cairo-dock-update.sh usr/bin
sudo cp -rp /usr/share/cairo-dock/ usr/share/
sudo rm -rf usr/share/cairo-dock/plug-in
sudo cp ../cairo-dock/data/cairo-dock.svg usr/share/pixmaps
sudo cp ../cairo-dock/data/cairo-dock usr/share/menu
sudo cp ../cairo-dock/data/cairo-dock.desktop usr/share/applications

cd $CAIRO_DOCK_DIR
sudo chmod -R 755 deb deb-plug-ins
mv deb/.svn ./.svn-deb
mv deb-plug-ins/.svn ./.svn-deb-plug-ins
mv deb/DEBIAN/.svn ./.svn-deb-DEIAN
mv deb-plug-ins/DEBIAN/.svn ./.svn-deb-plug-ins-DEIAN

dpkg -b deb "cairo-dock_`./cairo-dock/src/cairo-dock --version`_i686-32bits.deb"
mv .svn-deb deb/.svn
mv .svn-deb-DEIAN deb/DEBIAN/.svn

cd deb-plug-ins
sudo mkdir -p usr/share/cairo-dock
for lang in `cat ../cairo-dock/po/LINGUAS`; do
	sudo mkdir -p usr/share/locale/$lang/LC_MESSAGES
	sudo cp /usr/share/locale/$lang/LC_MESSAGES/cd-*.mo usr/share/locale/$lang/LC_MESSAGES
done;
sudo cp -r /usr/share/cairo-dock/plug-in usr/share/cairo-dock


cd $CAIRO_DOCK_DIR
dpkg -b deb-plug-ins "cairo-dock-plug-ins_`./cairo-dock/src/cairo-dock --version`_i686-32bits.deb"
mv .svn-deb-plug-ins deb-plug-ins/.svn
mv .svn-deb-plug-ins-DEIAN deb-plug-ins/DEBIAN/.svn


#\_____________ On liste les sommes de controle des fichiers.
rm -f md5sum.txt
for f in *.deb *.bz2; do echo `md5sum $f`>> md5sum.txt; done;
echo "generated files :"
echo "----------------"
cat md5sum.txt

#!/bin/sh

export CAIRO_DOCK_DIR=/opt/cairo-dock/trunk
export CAIRO_DOCK_FAST="0" 
export BUILD_TAR="0" 
export UNWANTED_APPLETS="xmms xfce-integration Cairo-Penguin tomboy mail wifi"
if test "$1" = "-f" -o "$2" = "-f"; then
	export CAIRO_DOCK_FAST="1"
fi
if test "$1" = "-t" -o "$2" = "-t"; then
	export BUILD_TAR="1"
fi

#\_____________ On nettoie tout et on fait une archive des sources.
cd $CAIRO_DOCK_DIR

find . -name "*~" -delete
find . -name "core*" -delete
find . -name ".#*" -delete

cd themes
./cairo-dock-finalize-theme.sh
cd ..

rm -f cairo-dock-sources*.tar.bz2 *.deb

cd cairo-dock
./compile-all.sh -ct

if test "$BUILD_TAR" = "1"; then
	cd /tmp
	tar cf `date +"cairo-dock-sources-%Y%m%d.tar"` $CAIRO_DOCK_DIR/cairo-dock $CAIRO_DOCK_DIR/plug-ins $CAIRO_DOCK_DIR/themes
	cd $CAIRO_DOCK_DIR
	mv `date +"/tmp/cairo-dock-sources-%Y%m%d.tar"` .
	bzip2 *.tar 
fi

#\_____________ On nettoie l'arborescence des paquets.
sudo rm -rf deb/usr
sudo rm -rf deb-plug-ins/usr

cd deb
if test -e debian; then
	mv debian DEBIAN
fi
cd ../deb-plug-ins
if test -e debian; then
	mv debian DEBIAN
fi

#\_____________ On compile de zero.
cd ../cairo-dock
if test "$CAIRO_DOCK_FAST" = "1"; then
	./compile-all.sh -C -i
else
	./compile-all.sh -a -C -i -t
fi

#\_____________ On cree les paquets.
cd ../deb
sudo mkdir usr
sudo mkdir usr/bin
sudo mkdir usr/share
sudo mkdir usr/share/menu
sudo mkdir usr/share/applications
sudo mkdir usr/share/pixmaps
for lang in `cat ../cairo-dock/po/LINGUAS`; do
	sudo mkdir -p usr/share/locale/$lang/LC_MESSAGES
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
dpkg -b deb "cairo-dock_`./cairo-dock/src/cairo-dock --version`_i686-32bits.deb"

cd deb-plug-ins
sudo mkdir -p usr/share/cairo-dock
for lang in `cat ../cairo-dock/po/LINGUAS`; do
	sudo mkdir -p usr/share/locale/$lang/LC_MESSAGES
done;
for lang in `cat ../cairo-dock/po/LINGUAS`; do
        sudo cp /usr/share/locale/$lang/LC_MESSAGES/cd-*.mo usr/share/locale/$lang/LC_MESSAGES
done;
sudo cp -r /usr/share/cairo-dock/plug-in usr/share/cairo-dock
for applet in $UNWANTED_APPLETS; do
	sudo rm -f "usr/share/cairo-dock/plug-in/libcd-$applet.so"
	sudo rm -rf "usr/share/cairo-dock/plug-in/$applet"
done;



cd $CAIRO_DOCK_DIR
dpkg -b deb-plug-ins "cairo-dock-plug-ins_`./cairo-dock/src/cairo-dock --version`_i686-32bits.deb"

#\_____________ On liste les sommes de controle des fichiers.
rm -f md5sum.txt
for f in *.deb *.bz2; do echo `md5sum $f`>> md5sum.txt; done;
echo "fichiers generes :"
echo "----------------"
cat md5sum.txt

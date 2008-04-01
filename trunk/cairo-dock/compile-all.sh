#!/bin/sh

export CAIRO_DOCK_DIR=/opt/cairo-dock/trunk
export CAIRO_DOCK_PREFIX=/usr
export CAIRO_DOCK_AUTORECONF="0"
export CAIRO_DOCK_CLEAN="0"
export CAIRO_DOCK_COMPIL="0"
export CAIRO_DOCK_UNSTABLE="0"
export CAIRO_DOCK_INSTALL="0"
export CAIRO_DOCK_THEMES="0"
export CAIRO_DOCK_EXCLUDE="template"
export CAIRO_DOCK_EXTRACT_MESSAGE=${CAIRO_DOCK_DIR}/utils/extract-message
export CAIRO_DOCK_GEN_TRANSLATION=${CAIRO_DOCK_DIR}/cairo-dock/po/generate-translation.sh

echo "this script will process : "
while getopts "acCituh" flag
do
	echo " option $flag" $OPTIND $OPTARG
	case "$flag" in
	a)
		echo " => re-generation of files"
		export CAIRO_DOCK_AUTORECONF="1"
		;;
	c)
		echo " => cleaning"
		export CAIRO_DOCK_CLEAN="1"
		;;
	C)
		echo " => compilation"
		export CAIRO_DOCK_COMPIL="1"
		;;
	i)
		echo " => installation"
		export CAIRO_DOCK_INSTALL="1"
		;;
	t)
		echo " => themes too"
		export CAIRO_DOCK_THEMES="1"
		;;
	u)
		echo " => include unstable applets"
		export CAIRO_DOCK_UNSTABLE="1"
		;;
	h)
		echo "-a : run autoreconf"
		echo "-c : clean all"
		echo "-C : compil"
		echo "-i : install (will ask root password)"
		echo "-t : compil themes too"
		echo "-u : include still unstable applets"
		exit 0
		;;
	*)
		echo "unexpected argument"
		;;
	esac
done
echo ""

cd $CAIRO_DOCK_DIR
find . -name linguas -execdir mv linguas LINGUAS \;
find . -name potfiles.in -execdir mv potfiles.in POTFILES.in \;


cd cairo-dock
if test "$CAIRO_DOCK_CLEAN" = "1"; then
	rm -f config.* configure configure.lineno intltool-extract intltool-merge intltool-update libtool ltmain.sh Makefile.in Makefile aclocal.m4 install-sh install depcomp missing compile cairo-dock.pc stamp-h1 cairo-dock.conf 
	rm -rf autom4te.cache src/.deps src/.libs src/Makefile src/Makefile.in po/Makefile po/Makefile.in po/*.gmo src/*.o src/*.lo src/*.la
fi
if test "$CAIRO_DOCK_AUTORECONF" = "1"; then
	if test -e po; then
		if test -x $CAIRO_DOCK_EXTRACT_MESSAGE; then
			for c in data/*.conf.in
			do
				$CAIRO_DOCK_EXTRACT_MESSAGE $c
			done;
		fi
		cd po
		$CAIRO_DOCK_GEN_TRANSLATION
		cd ..
	fi
	autoreconf -isvf && ./configure --prefix=$CAIRO_DOCK_PREFIX --enable-glitz
fi
if test "$CAIRO_DOCK_CLEAN" = "1" -a -e Makefile; then
	make clean
fi
if test "$CAIRO_DOCK_COMPIL" = "1"; then
	make
fi
if test "$CAIRO_DOCK_INSTALL" = "1"; then
	echo "installation de cairo-dock..."
	if test "$CAIRO_DOCK_CLEAN" = "1"; then
		sudo rm -f $CAIRO_DOCK_PREFIX/bin/cairo-dock
		sudo rm -rf $CAIRO_DOCK_PREFIX/share/cairo-dock
	fi
	sudo make install
	sudo chmod +x $CAIRO_DOCK_PREFIX/bin/cairo-dock-update.sh
	sudo chmod +x $CAIRO_DOCK_PREFIX/bin/launch-cairo-dock-after-beryl.sh
fi


cd ..
if test "$CAIRO_DOCK_THEMES" = "1"; then
	cd themes
	if test "$CAIRO_DOCK_CLEAN" = "1"; then
		rm -f config.* configure configure.lineno intltool-extract intltool-merge intltool-update libtool ltmain.sh Makefile.in Makefile aclocal.m4 install-sh install depcomp missing compile stamp-h1 autom4te.cache
		find . -name Makefile -delete
		find . -name Makefile.in -delete
	fi
	if test "$CAIRO_DOCK_AUTORECONF" = "1"; then
		autoreconf -isvf && ./configure
	fi
	if test "$CAIRO_DOCK_COMPIL" = "1"; then
		make
	fi
	if test "$CAIRO_DOCK_INSTALL" = "1"; then
		echo "installation des themes ..."
		sudo make install
	fi
	cd ..
fi


cd plug-ins
if test "$CAIRO_DOCK_UNSTABLE" = "1" -o ! -e "Applets.stable"; then
	export liste="`ls`"
else
	export liste="`sed "/^#/d" Applets.stable`"
fi
echo "the following applets will be compiled :"
echo "$liste"
for plugin in $liste
do
	if test -d $plugin; then
		cd $plugin
		if test -e Makefile.am -a "$plugin" != "$CAIRO_DOCK_EXCLUDE"; then
			echo "**********************************"
			echo "* Compilation du module $plugin ... *"
			echo "**********************************"
			if test "$CAIRO_DOCK_CLEAN" = "1"; then
				rm -f config.* configure configure.lineno intltool-extract intltool-merge intltool-update libtool ltmain.sh Makefile.in Makefile aclocal.m4 missing stamp-h1 depcomp compile
				rm -rf autom4te.cache src/.deps src/.libs src/Makefile src/Makefile.in po/Makefile po/Makefile.in po/*.gmo src/*.o src/*.lo src/*.la
			fi
			if test "$CAIRO_DOCK_AUTORECONF" = "1"; then
				if test -e po; then
					if test -x $CAIRO_DOCK_EXTRACT_MESSAGE; then
						for c in data/*.conf.in
						do
							$CAIRO_DOCK_EXTRACT_MESSAGE $c
						done;
					fi
					cd po
					$CAIRO_DOCK_GEN_TRANSLATION
					cd ..
				fi
				autoreconf -isvf && ./configure --prefix=$CAIRO_DOCK_PREFIX --enable-glitz
			fi
			if test "$CAIRO_DOCK_CLEAN" = "1" -a -e Makefile; then
				make clean
			fi
			if test "$CAIRO_DOCK_COMPIL" = "1"; then
				make
			fi
			if test "$CAIRO_DOCK_INSTALL" = "1"; then
				echo "installation du module $plugin..."
				sudo make install
			fi
		fi
		cd ..
	fi
done;

echo "fini !"

if test "$CAIRO_DOCK_INSTALL" = "1"; then
	echo "verification :"
	echo "------------"
	date +"compil ended at %c"
	ls -l $CAIRO_DOCK_PREFIX/bin/cairo-dock
	ls -l $CAIRO_DOCK_PREFIX/share/cairo-dock/plug-in
fi

exit

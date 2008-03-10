#!/bin/sh

export CAIRO_DOCK_DIR=/opt/cairo-dock/trunk
export CAIRO_DOCK_PREFIX=/usr
export CAIRO_DOCK_AUTORECONF="0"
export CAIRO_DOCK_CLEAN="0"
export CAIRO_DOCK_COMPIL="0"
export CAIRO_DOCK_INSTALL="0"
export CAIRO_DOCK_THEMES="0"
export CAIRO_DOCK_EXCLUDE="template"
export CAIRO_DOCK_EXTRACT_MESSAGE=${CAIRO_DOCK_DIR}/utils/extract-message
export CAIRO_DOCK_GEN_TRANSLATION=${CAIRO_DOCK_DIR}/cairo-dock/po/generate-translation.sh

while getopts "acCit" flag
do
	echo "$flag" $OPTIND $OPTARG
	case "$flag" in
	a)
		export CAIRO_DOCK_AUTORECONF="1"
		;;
	c)
		export CAIRO_DOCK_CLEAN="1"
		;;
	C)
		export CAIRO_DOCK_COMPIL="1"
		;;
	i)
		export CAIRO_DOCK_INSTALL="1"
		;;
	t)
		export CAIRO_DOCK_THEMES="1"
		;;
	*)
		echo "unexpected argument"
		;;
	esac
done


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
for plugin in *
do
	if test -d $plugin; then
		cd $plugin
		if test -e Makefile.am -a "$plugin" != "$CAIRO_DOCK_EXCLUDE"; then
			echo "********************************"
			echo "* Compilation du module $plugin ... *"
			echo "********************************"
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
	date +"compil finie a %T le %D"
	ls -l $CAIRO_DOCK_PREFIX/bin/cairo-dock
	ls -l $CAIRO_DOCK_PREFIX/share/cairo-dock/plug-in
fi

exit

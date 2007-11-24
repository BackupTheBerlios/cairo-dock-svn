#!/bin/bash

DIR=$(pwd)
LOG=$DIR/log.txt

PLUGINS="file-manager file-manager-gnome clock dustbin logout rhythmbox rendering"
NEEDED="cvs libtool build-essential automake1.9 autoconf m4 autotools-dev pkg-config libtool libcairo2-dev libgtk2.0-dev librsvg2-dev libglitz1-dev libcairo2 librsvg2-2 libglitz1 libglitz-glx1 libglitz-glx1-dev libdbus-glib-1-dev libgnomevfs2-dev libgnomeui-dev"

MAJ=0
UPDATE=0

NORMAL="\\033[0;39m"
BLEU="\\033[1;34m"
VERT="\\033[1;32m" 
ROUGE="\\033[1;31m"

install_cairo_dock() {

	echo -e "$BLEU""Installation de cairo-dock"
	install_cairo >> $LOG 2>&1
	if [ $? -gt 0 ]; then 
		echo -e "$ROUGE""Attention : problème lors de l'installation de cairo-dock"
		echo -e "$NORMAL"
		exit 1
	else 
		echo -e "$VERT""OK"
	fi
	echo -e "$NORMAL"

	for plugins in $PLUGINS
	do
		echo -e "$BLEU""Installation du plug-in $plugins"
		install_plugins "$plugins" >> $LOG 2>&1
		if [ $? -gt 0 ]; then 
			echo -e "$ROUGE""Attention : problème lors de l'installation du plugin $plugins"
			echo -e "$NORMAL"
			exit 1
		else 
			echo -e "$VERT""OK"
		fi
		echo -e "$NORMAL"
	done

}


install_cairo() {

	cd $DIR/cairo-dock
	autoreconf -isvf && ./configure --prefix=/usr && make
	sudo make install
	cd ..
}

install_plugins() {
	
	cd $DIR/plug-ins
	cd $1
	autoreconf -isvf && ./configure --prefix=/usr && make
	sudo make install
	cd ..
				
}


check() {
	grep -q '^Erreur' $LOG
	if [ $? -eq 0 ]; then
		echo -e "$ROUGE"
		echo "Problèmes lors de l'installation : veuillez consulter le fichier log.txt"
		echo -e "$NORMAL"
	else
		echo -e "$VERT"
		echo "L'installation s'est terminée correctement."
		echo -e "$NORMAL"
	fi
     	exit
}


test "$1" == "?"
if [ $? -eq 0 ]
then
        echo "Usage :"
        echo "$0 --force-install" #Si vous avez effectué des modifications dans le CVS par vous-même et souhaitez installer cairo-dock
        echo "$0" # Pour mettre à jour le CVS et installer cairo-dock
        exit
fi


test "$1" == "--force-install"
if [ $? -eq 0 ]; then
     	install_cairo_dock
	check
fi

echo ""
echo -e "$BLEU""Vérification des paquets nécéssaires à la compilation" 

for tested in $NEEDED
do
	dpkg -s $tested |grep installed |grep "install ok" > /dev/null	
	if [ $? -eq 1 ]; then
		echo -e "$ROUGE""Le paquet $tested n'est pas installé""$NORMAL"""
		sudo apt-get install $tested
		
	fi
done
echo ""
echo -e "$VERT""Vérification OK""$NORMAL"""


if [ -d $DIR ]; then
	echo
else
	mkdir -p $DIR
fi

export CVSROOT=$DIR
cd $DIR

if [ -d cairo-dock ]; then
	if [ -d plug-ins ]; then
		UPDATE=1
	fi
fi

cvs -qd:pserver:anonymous@cvs.cairo-dock.berlios.de:/cvsroot/cairo-dock login

if [ $UPDATE -eq 1 ]; then

	for i in `cvs update -dP . | grep '^[UPM]'`; do
		MAJ=1
		echo ""
		echo -e "$BLEU""Mise à jour détéctée, installation de la nouvelle version"
		echo "Cette opération peut prendre plusieurs minutes et ralentir votre système"
		echo -e "$NORMAL"
		sleep 2
	
		install_cairo_dock
		check
		
		exit 1

	done


else
	echo -e "$BLEU"
	echo "C'est la première fois que vous installez le CVS de Cairo-Dock"
	echo ""
	echo "Téléchargement des données. Ceci peut prendre quelques minutes"
	echo -e "$NORMAL"

	mkdir -p $CVSROOT/CVSROOT
	cvs -z3 -d:pserver:anonymous@cvs.cairo-dock.berlios.de:/cvsroot/cairo-dock co .
	if [ $? -gt 0 ]; then 
		echo -e "$ROUGE"
		echo "Attention : impossible de récupérer les mises à jour. Retentez plus tard."
		echo -e "$NORMAL"		
		exit
	fi
	
	echo ""
	echo -e "$BLEU""Données téléchargées. L'installation va débuter"
	echo "Cette opération peut prendre plusieurs minutes et ralentir votre système"
	echo -e "$NORMAL"

	sleep 5
	
	install_cairo_dock
	check
	
fi

if [ $MAJ -eq 0 ]; then
	echo -e "$BLEU"
	echo "Pas de mise à jour disponible"
	echo -e "$NORMAL"
fi




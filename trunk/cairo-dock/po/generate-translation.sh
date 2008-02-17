#!/bin/sh

export lang="$1"
if test "$lang" = ""; then
	export lang="fr"
fi

for l in `cat LINGUAS`
do
	xgettext -L C -k_ -k_D -d $lang -p . -j ${lang}.po --from-code=UTF-8 --omit-header --copyright-holder="Cairo-Dock project" --msgid-bugs-address="fabounet@users.berlios.de"  ../src/*.c ../data/messages
	#xgettext -L Shell -k_ -a -d $lang -p po -j ${lang}.po --from-code=UTF-8 --omit-header --copyright-holder="Cairo-Dock project" --msgid-bugs-address="fabounet@users.berlios.de"  ../data/cairo-dock.conf
done;

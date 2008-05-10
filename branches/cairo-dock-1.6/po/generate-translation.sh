#!/bin/sh

for lang in `cat LINGUAS`
do
	xgettext -L C -k_ -k_D -kD_ -kN_ -d $lang -p . -j ${lang}.po --from-code=UTF-8 --omit-header --copyright-holder="Cairo-Dock project" --msgid-bugs-address="fabounet@users.berlios.de"  ../src/*.c ../data/messages
done;

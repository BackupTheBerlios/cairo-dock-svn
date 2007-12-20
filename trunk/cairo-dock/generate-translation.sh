#!/bin/sh

export lang="fr"

xgettext -L C -k_ -d $lang -p po -j po/${lang}.po --from-code=UTF-8 --omit-header --copyright-holder="Cairo-Dock project" --msgid-bugs-address="fabounet@users.berlios.de"  src/*.c
#xgettext -L Shell -k_ -a -d $lang -p po -j po/${lang}.po --from-code=UTF-8 --omit-header --copyright-holder="Cairo-Dock project" --msgid-bugs-address="fabounet@users.berlios.de"  data/cairo-dock-en.conf

#!/bin/sh

find . -type l -delete

for t in `ls` ../cairo-dock/data/default-theme
do
	if test -d $t -a -e $t/Makefile.am; then
		echo "normalisation du theme $t ..."
		cd $t/launchers
		for suff in "svg" "png"
		do
			echo "  creation des liens $suff ..."
			
			if test -e web-browser.$suff; then
				echo "    vers web-browser.$suff"
				ln -s web-browser.$suff firefox.$suff
				ln -s web-browser.$suff opera.$suff
				ln -s web-browser.$suff epiphany.$suff
			fi
			ln -s firefox.$suff firefox-3.0.$suff
			if test -e file-browser.$suff; then
				echo "    vers file-browser.$suff"
				ln -s file-browser.$suff nautilus.$suff
				ln -s file-browser.$suff konqueror.$suff
				ln -s file-browser.$suff thunar.$suff
				ln -s file-browser.$suff pcmanfm.$suff
			fi
			
			if test -e mail-reader.$suff; then
				echo "    vers mail-reader.$suff"
				ln -s mail-reader.$suff mozilla-thunderbird.$suff
				ln -s mail-reader.$suff kmail.$suff
				ln -s mail-reader.$suff evolution.$suff
			fi
			if test -e image-reader.$suff; then
				echo "    vers image-reader.$suff"
				ln -s image-reader.$suff eog.$suff
				ln -s image-reader.$suff gqview.$suff
				ln -s image-reader.$suff gwenview.$suff
				ln -s image-reader.$suff f-spot.$suff
			fi
			if test -e audio-player.$suff; then
				echo "    vers audio-player.$suff"
				ln -s audio-player.$suff xmms.$suff
				ln -s audio-player.$suff bmp.$suff
				ln -s audio-player.$suff beep-media-player.$suff
				ln -s audio-player.$suff rhythmbox.$suff
				ln -s audio-player.$suff amarok.$suff
			fi
			if test -e video-player.$suff; then
				echo "    vers video-player.$suff"
				ln -s video-player.$suff totem.$suff
				ln -s video-player.$suff mplayer.$suff
				ln -s video-player.$suff vlc.$suff
				ln -s video-player.$suff xine.$suff
				ln -s video-player.$suff kaffeine.$suff
			fi
			
			if test -e writer.$suff; then
				echo "    vers writer.$suff"
				ln -s writer.$suff gedit.$suff
				ln -s writer.$suff kate.$suff
				ln -s writer.$suff ooo-writer.$suff
				ln -s writer.$suff abiword.$suff
				ln -s writer.$suff emacs.$suff
			fi
			if test -e bittorrent.$suff; then
				echo "    vers bittorrent.$suff"
				ln -s bittorrent.$suff transmission.$suff
				ln -s bittorrent.$suff deluge.$suff
				ln -s bittorrent.$suff bittornado.$suff
				ln -s bittorrent.$suff gnome-btdownload.$suff
				ln -s bittorrent.$suff ktorrent.$suff
			fi
			if test -e download.$suff; then
				echo "    vers download.$suff"
				ln -s download.$suff amule.$suff
				ln -s download.$suff emule.$suff
				ln -s download.$suff filezilla.$suff
			fi
			if test -e cd-burner.$suff; then
				echo "    vers cd-burner.$suff"
				ln -s cd-burner.$suff nautilus-cd-burner.$suff
				ln -s cd-burner.$suff graveman.$suff
				ln -s cd-burner.$suff gnome-baker.$suff
				ln -s cd-burner.$suff k3b.$suff
				ln -s cd-burner.$suff brasero.$suff
			fi
			if test -e image.$suff; then
				echo "    vers image.$suff"
				ln -s image.$suff gimp.$suff
				ln -s image.$suff inkscape.$suff
				ln -s image.$suff krita.$suff
			fi
			
			if test -e messenger.$suff; then
				echo "    vers messenger.$suff"
				ln -s messenger.$suff gaim.$suff
				ln -s messenger.$suff pidgin.$suff
				ln -s messenger.$suff kopete.$suff
				ln -s messenger.$suff amsn.$suff
				ln -s messenger.$suff emessene.$suff
			fi
			if test -e irc.$suff; then
				echo "    vers irc.$suff"
				ln -s irc.$suff xchat.$suff
				ln -s irc.$suff konversation.$suff
				ln -s irc.$suff kvirc.$suff
			fi
			
			if test -e terminal.$suff; then
				echo "    vers terminal.$suff"
				ln -s terminal.$suff gnome-terminal.$suff
				ln -s terminal.$suff konsole.$suff
				ln -s terminal.$suff xfce4-terminal.$suff
			fi
			if test -e packages.$suff; then
				echo "    vers packages.$suff"
				ln -s packages.$suff synaptic.$suff
				ln -s packages.$suff adept.$suff
				ln -s packages.$suff pacman-g2.$suff
			fi
			if test -e system-monitor.$suff; then
				echo "    vers system-monitor.$suff"
				ln -s system-monitor.$suff ksysguard.$suff
				ln -s system-monitor.$suff utilities-system-monitor.$suff
			fi
			if test -e calculator.$suff; then
				echo "    vers calculator.$suff"
				ln -s calculator.$suff gnome-calculator.$suff
				ln -s calculator.$suff crunch.$suff
			fi
		done;
		cd ../..
	fi
done;

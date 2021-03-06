#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([cairo-dock-plugins],[2.1.0-alpha],[Fabrice Rey (fabounet@users.berlios.de)])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign])
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

IT_PROG_INTLTOOL()
AC_SUBST(ALL_LINGUAS)
AM_GLIB_GNU_GETTEXT


############# PACKAGE CAIRO-DOCK #################
GETTEXT_PACKAGE=cairo-dock-plugins
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"GETTEXT_PACKAGE", [Gettext package.])
AC_SUBST(GETTEXT_PACKAGE)

pkgdatadir=[`pkg-config cairo-dock --variable=pluginsdatadir`]
AC_SUBST(pkgdatadir)
libdir=[`pkg-config cairo-dock --variable=pluginsdir`]
AC_SUBST(libdir)

srcdir="."
AC_SUBST(srcdir)
top_srcdir=".."
AC_SUBST(top_srcdir)
top_builddir=".."
AC_SUBST(top_builddir)

PKG_CHECK_MODULES(PACKAGE, "cairo-dock gtk+-2.0 gthread-2.0 cairo librsvg-2.0 dbus-1 dbus-glib-1 libxml-2.0")


############# COMPILATION CONDITIONNELLE #################
AC_ARG_ENABLE([old-gnome-integration],
	[  --enable-old-gnome-integration : Enable Gnome<2.22 integration (default=yes) ],
	[ with_old_gnome_support=${enableval} ],
	[ with_old_gnome_support=yes ])
AC_ARG_ENABLE(gnome-integration,
	[  --enable-gnome-integration : Enable Gnome>=2.22 integration (default=yes) ],
	[ with_gnome_support=${enableval} ],
	[ with_gnome_support=yes ])
AC_ARG_ENABLE(xfce-integration,
	[  --enable-xfce-integration : Enable XFCE integration (default=yes) ],
	[ with_xfce_support=${enableval} ],
	[ with_xfce_support=yes ])
AC_ARG_ENABLE(alsa-mixer,
	[  --enable-alsa-mixer : Enable Alsa-Mixer applet (default=yes) ],
	[ with_alsa_mixer=${enableval} ],
	[ with_alsa_mixer=yes ])
AC_ARG_ENABLE(terminal,
	[  --enable-terminal : Enable Terminal applet (default=yes) ],
	[ with_terminal=${enableval} ],
	[ with_terminal=yes ])
AC_ARG_ENABLE(powermanager,
	[  --enable-powermanager : Enable PowerManager applet (default=yes) ],
	[ with_powermanager=${enableval} ],
	[ with_powermanager=yes ])
AC_ARG_ENABLE(wifi,
	[  --enable-wifi : Enable Wifi applet (default=yes) ],
	[ with_wifi=${enableval} ],
	[ with_wifi=yes ])
AC_ARG_ENABLE(tomboy,
	[  --enable-tomboy : Enable Tomboy applet (default=yes) ],
	[ with_tomboy=${enableval} ],
	[ with_tomboy=yes ])
AC_ARG_ENABLE(compiz-icon,
	[  --enable-compiz-icon : Enable Compiz-Icon applet (default=yes) ],
	[ with_compiz_icon=${enableval} ],
	[ with_compiz_icon=yes ])
AC_ARG_ENABLE(xgamma,
	[  --enable-xgamma : Enable Xgamma applet (default=yes) ],
	[ with_xgamma=${enableval} ],
	[ with_xgamma=yes ])
AC_ARG_ENABLE(gmenu,
	[  --enable-gmenu : Enable GMenu applet (default=yes) ],
	[ with_gmenu=${enableval} ],
	[ with_gmenu=yes ])
AC_ARG_ENABLE(gio-in-gmenu,
        [  --enable-gio-in-gmenu : Use gio in GMenu (default=no) ],
        [ with_gio_in_gmenu=${enableval} ],
        [ with_gio_in_gmenu=no])
AC_ARG_ENABLE(mail,
	[  --enable-mail : Enable Mail applet (default=no) ],
	[ with_mail=${enableval} ],
	[ with_mail=no ])
AC_ARG_ENABLE(weblets,
	[  --enable-weblets : Enable Weblets applet (default=yes) ],
	[ with_weblets=${enableval} ],
	[ with_weblets=yes ])
AC_ARG_ENABLE(exif,
	[  --enable-exif : for auto-rotating pictures in Slider (default=yes) ],
	[ with_exif=${enableval} ],
	[ with_exif=yes ])
AC_ARG_ENABLE(keyboard-indicator,
	[  --enable-keyboard-indicator : Enable keyboard-indicator applet (default=yes) ],
	[ with_keyboard_indicator=${enableval} ],
	[ with_keyboard_indicator=yes ])
AC_ARG_ENABLE(dnd2share,
	[  --enable-dnd2share : Enable dnd2share applet (default=yes) ],
	[ with_dnd2share=${enableval} ],
	[ with_dnd2share=yes ])
AC_ARG_ENABLE(musicplayer,
	[  --enable-musicplayer : Enable musicplayer applet (default=yes) ],
	[ with_musicplayer=${enableval} ],
	[ with_musicplayer=yes ])


############# PACKAGES ADDITIONNELS #################
if test "$with_old_gnome_support" = "yes" ; then
	PKG_CHECK_MODULES(OLD_GNOME_INTEGRATION,gnome-vfs-2.0 libgnomeui-2.0, ,[ with_old_gnome_support=false; AC_MSG_WARN(Could not find gnome-vfs and/or gnomeui; Cairo-Dock won\'t be built with Gnome<2.22 support.) ])
	AC_SUBST(OLD_GNOME_INTEGRATION_CFLAGS)
	AC_SUBST(OLD_GNOME_INTEGRATION_LIBS)
fi
if test "$with_gnome_support" = "yes" ; then
	PKG_CHECK_MODULES(GNOME_INTEGRATION,gio-2.0, ,[ with_gnome_support=false; AC_MSG_WARN(Could not find gio; Cairo-Dock won\'t be built with Gnome>=2.22 support.) ])
	AC_SUBST(GNOME_INTEGRATION_CFLAGS)
	AC_SUBST(GNOME_INTEGRATION_LIBS)
fi
if test "$with_xfce_support" = "yes" ; then
	PKG_CHECK_MODULES(XFCE_INTEGRATION,thunar-vfs-1, ,[ with_xfce_support=false; AC_MSG_WARN(Could not find thunar-vfs; Cairo-Dock won\'t be built with XFCE support.) ])
	AC_SUBST(XFCE_INTEGRATION_CFLAGS)
	AC_SUBST(XFCE_INTEGRATION_LIBS)
fi
if test "$with_alsa_mixer" = "yes" ; then
	PKG_CHECK_MODULES(ALSA_MIXER_PACKAGE,alsa, ,[ with_alsa_mixer=false; AC_MSG_WARN(Could not find alsa; Cairo-Dock won\'t be built with AlsaMixer applet.) ])
	AC_SUBST(ALSA_MIXER_PACKAGE_CFLAGS)
	AC_SUBST(ALSA_MIXER_PACKAGE_LIBS)
fi
if test "$with_terminal" = "yes" ; then
	PKG_CHECK_MODULES(TERMINAL_PACKAGE,vte, ,[ with_terminal=false; AC_MSG_WARN(Could not find vte; Cairo-Dock won\'t be built with Terminal applet.) ])
	AC_SUBST(TERMINAL_PACKAGE_CFLAGS)
	AC_SUBST(TERMINAL_PACKAGE_LIBS)
fi
if test "$with_xgamma" = "yes" ; then
	PKG_CHECK_MODULES(XGAMMA_PACKAGE,x11 xxf86vm, ,[ with_xgamma=false; AC_MSG_WARN(Could not find xxf86vm; Cairo-Dock won\'t be built with Xgamma applet.) ])
	AC_SUBST(XGAMMA_PACKAGE_CFLAGS)
	AC_SUBST(XGAMMA_PACKAGE_LIBS)
fi
if test "$with_gmenu" = "yes" ; then
	PKG_CHECK_MODULES(GMENU_PACKAGE,libgnome-menu, ,[ with_gmenu=false; AC_MSG_WARN(Could not find libgnome-menu; Cairo-Dock won\'t be built with GMenu applet.) ])
	AC_SUBST(GMENU_PACKAGE_CFLAGS)
	AC_SUBST(GMENU_PACKAGE_LIBS)
fi
if test "$with_gio_in_gmenu" = "yes" ; then
	PKG_CHECK_MODULES(GMENU_GIO, gio-2.0, ,[ with_gio_in_gmenu=false; AC_MSG_WARN(Could not find gio-2.0; the GMenu applet won\'t be built with. Depending on libgnome-menu, it may not be a problem.) ])
	AC_SUBST(GMENU_GIO_CFLAGS)
	AC_SUBST(GMENU_GIO_LIBS)
fi
if test "$with_mail" = "yes" ; then
	MAIL_PACKAGE_CFLAGS=`libetpan-config --cflags`
	MAIL_PACKAGE_LIBS=`libetpan-config --libs`
	if test "x$MAIL_PACKAGE_LIBS" = "x"; then
		echo "warning : Could not find libetpan; Cairo-Dock won't be built with Mail applet."
		with_mail=false
	fi
	AC_SUBST(MAIL_PACKAGE_CFLAGS)
	AC_SUBST(MAIL_PACKAGE_LIBS)
fi
if test "$with_weblets" = "yes" ; then
	PKG_CHECK_MODULES(WEBKIT,webkit-1.0, ,[ with_weblets=false; AC_MSG_WARN(Could not find webkit-1.0; Cairo-Dock won\'t be built with Weblets applet.) ])
	AC_SUBST(WEBKIT_CFLAGS)
	AC_SUBST(WEBKIT_LIBS)
fi
if test "$with_exif" = "yes" ; then
	PKG_CHECK_MODULES(EXIF,libexif, ,[ with_exif=false; AC_MSG_WARN(Could not find libexif; Slider won\'t be built with exif capbilities.) ])
	AC_SUBST(EXIF_CFLAGS)
	AC_SUBST(EXIF_LIBS)
fi
if test "$with_keyboard_indicator" = "yes" ; then
	PKG_CHECK_MODULES(KEYBOARD_INDICATOR_PACKAGE,libxklavier, ,[ with_keyboard_indicator=false; AC_MSG_WARN(Could not find libxklavier; Cairo-Dock won\'t be built with keyboard-indicator applet.) ])
	AC_SUBST(KEYBOARD_INDICATOR_PACKAGE_CFLAGS)
	AC_SUBST(KEYBOARD_INDICATOR_PACKAGE_LIBS)
fi

AC_PROG_CXX

AM_CONDITIONAL( [WITH_OLD_GNOME_SUPPORT], [test "$with_old_gnome_support" = "yes"] )
AM_CONDITIONAL( [WITH_GNOME_SUPPORT], [test "$with_gnome_support" = "yes"] )
AM_CONDITIONAL( [WITH_XFCE_SUPPORT], [test "$with_xfce_support" = "yes"] )
AM_CONDITIONAL( [WITH_ALSA_MIXER], [test "$with_alsa_mixer" = "yes"] )
AM_CONDITIONAL( [WITH_TERMINAL], [test "$with_terminal" = "yes"] )
AM_CONDITIONAL( [WITH_POWERMANAGER], [test "$with_powermanager" = "yes"] )
AM_CONDITIONAL( [WITH_WIFI], [test "$with_wifi" = "yes"] )
AM_CONDITIONAL( [WITH_TOMBOY], [test "$with_tomboy" = "yes"] )
AM_CONDITIONAL( [WITH_COMPIZ_ICON], [test "$with_compiz_icon" = "yes"] )
AM_CONDITIONAL( [WITH_XGAMMA], [test "$with_xgamma" = "yes"] )
AM_CONDITIONAL( [WITH_GMENU], [test "$with_gmenu" = "yes"] )
AM_CONDITIONAL( [WITH_GIO_IN_GMENU], [test "$with_gio_in_gmenu" = "yes"] )
AM_CONDITIONAL( [WITH_MAIL], [test "$with_mail" = "yes"] )
AM_CONDITIONAL( [WITH_WEBLETS], [test "$with_weblets" = "yes"] )
AM_CONDITIONAL( [WITH_EXIF], [test "$with_exif" = "yes"] )
AM_CONDITIONAL( [WITH_KEYBOARD_INDICATOR], [test "$with_keyboard_indicator" = "yes"] )
AM_CONDITIONAL( [WITH_DND2SHARE], [test "$with_dnd2share" = "yes"] )
AM_CONDITIONAL( [WITH_MUSICPLAYER], [test "$with_musicplayer" = "yes"] )


############# ALSA_MIXER #################
ALSA_MIXER_PACKAGE="cd-AlsaMixer"
AC_SUBST(GETTEXT_ALSA_MIXER, "cairo-dock-plugins")

AC_SUBST(VERSION_ALSA_MIXER, "1.0.9")
AC_SUBST(PACKAGE_ALSA_MIXER, "$ALSA_MIXER_PACKAGE")

AC_SUBST(alsa_mixerdatadir, "${pkgdatadir}/AlsaMixer")

############# ANIMATED ICONS #################
ANIMATED_ICONS_PACKAGE=cd-Animated-icons
AC_SUBST(GETTEXT_ANIMATED_ICONS, "cairo-dock-plugins")

AC_SUBST(VERSION_ANIMATED_ICONS, "1.0.4")
AC_SUBST(PACKAGE_ANIMATED_ICONS, "$ANIMATED_ICONS_PACKAGE")

AC_SUBST(animated_iconsdatadir, "${pkgdatadir}/Animated-icons")

############# CLIPPER #################
CLIPPER_PACKAGE="cd-Clipper"
AC_SUBST(GETTEXT_CLIPPER, "cairo-dock-plugins")

AC_SUBST(VERSION_CLIPPER, "1.1.0")
AC_SUBST(PACKAGE_CLIPPER, "$CLIPPER_PACKAGE")

AC_SUBST(Clipperdatadir, "${pkgdatadir}/Clipper")

############# CAIRO_PENGUIN #################
CAIRO_PENGUIN_PACKAGE="cd-Cairo-Penguin"
AC_SUBST(GETTEXT_CAIRO_PENGUIN, "cairo-dock-plugins")

AC_SUBST(VERSION_CAIRO_PENGUIN, "1.2.0")
AC_SUBST(PACKAGE_CAIRO_PENGUIN, "$CAIRO_PENGUIN_PACKAGE")
cairo_penguinuserdirname="Cairo-Penguin"
AC_SUBST(cairo_penguinuserdirname)

AC_SUBST(cairo_penguindatadir, "${pkgdatadir}/Cairo-Penguin")

############# CLOCK #################
CLOCK_PACKAGE="cd-clock"
AC_SUBST(GETTEXT_CLOCK, "cairo-dock-plugins")

AC_SUBST(VERSION_CLOCK, "2.0.5")
AC_SUBST(PACKAGE_CLOCK, "$CLOCK_PACKAGE")
clockuserdirname="clock"
AC_SUBST(clockuserdirname, "clock")

AC_SUBST(clockdatadir, "${pkgdatadir}/clock")

############# COMPIZ_ICON #################
COMPIZ_ICON_PACKAGE="cd-compiz-icon"
AC_SUBST(GETTEXT_COMPIZ_ICON, "cairo-dock-plugins")

AC_SUBST(VERSION_COMPIZ_ICON, "1.1.3")
AC_SUBST(PACKAGE_COMPIZ_ICON, "$COMPIZ_ICON_PACKAGE")

AC_SUBST(compiz_icondatadir, "${pkgdatadir}/compiz-icon")

############# DBUS #################
DBUS_PACKAGE="cd-Dbus"
AC_SUBST(GETTEXT_DBUS, "cairo-dock-plugins")

AC_SUBST(VERSION_DBUS, "0.6.0")
AC_SUBST(PACKAGE_DBUS, "$DBUS_PACKAGE")

AC_SUBST(dbusdatadir, "${pkgdatadir}/Dbus")

############# DND2SHARE #################
DND2SHARE_PACKAGE="cd-dnd2share"
AC_SUBST(GETTEXT_DND2SHARE, "cairo-dock-plugins")

AC_SUBST(VERSION_DND2SHARE, "1.0.0")
AC_SUBST(PACKAGE_DND2SHARE, "$DBUS_PACKAGE")

AC_SUBST(dnd2sharedatadir, "${pkgdatadir}/dnd2share")

############# DROP INDICATOR #################
DROP_INDICATOR_PACKAGE=cd-drop_indicator
AC_SUBST(GETTEXT_DROP_INDICATOR, "cairo-dock-plugins")

AC_SUBST(VERSION_DROP_INDICATOR, "1.1.1")
AC_SUBST(PACKAGE_DROP_INDICATOR, "$DROP_INDICATOR_PACKAGE")

AC_SUBST(drop_indicatordatadir, "${pkgdatadir}/drop-indicator")

############# DUSTBIN #################
DUSTBIN_PACKAGE=cd-dustbin
AC_SUBST(GETTEXT_DUSTBIN, "cairo-dock-plugins")

AC_SUBST(VERSION_DUSTBIN, "2.2.4")
AC_SUBST(PACKAGE_DUSTBIN, "$DUSTBIN_PACKAGE")
dustbinuserdirname="dustbin"
AC_SUBST(dustbinuserdirname)


AC_SUBST(dustbindatadir, "${pkgdatadir}/dustbin")

############# GMENU #################
GMENU_PACKAGE=cd-GMenu
AC_SUBST(GETTEXT_GMENU, "cairo-dock-plugins")

AC_SUBST(VERSION_GMENU, "1.1.3")
AC_SUBST(PACKAGE_GMENU, "$GMENU_PACKAGE")

AC_SUBST(gmenudatadir, "${pkgdatadir}/GMenu")

############# GNOME-INTEGRATION #################
GNOME_INTEGRATION_PACKAGE=cd_gnome-integration

AC_SUBST(VERSION_GNOME_INTEGRATION, "1.0.2")
AC_SUBST(PACKAGE_GNOME_INTEGRATION, "$GNOME_INTEGRATION_PACKAGE")

AC_SUBST(gnome_integrationdatadir, "${pkgdatadir}/gnome-integration")

############# GNOME-INTEGRATION-OLD #################
GNOME_INTEGRATION_OLD_PACKAGE=cd_gnome-integration-old

AC_SUBST(VERSION_GNOME_INTEGRATION_OLD, "1.0.3")
AC_SUBST(PACKAGE_GNOME_INTEGRATION_OLD, "$GNOME_INTEGRATION_OLD_PACKAGE")

AC_SUBST(gnome_integration_olddatadir, "${pkgdatadir}/gnome-integration-old")

############# ICON EFFECT #################
ICON_EFFECTS_PACKAGE=cd-icon-effect
AC_SUBST(GETTEXT_ICON_EFFECTS, "cairo-dock-plugins")

AC_SUBST(VERSION_ICON_EFFECTS, "1.1.4")
AC_SUBST(PACKAGE_ICON_EFFECTS, "$ICON_EFFECTS_PACKAGE")

AC_SUBST(icon_effectsdatadir, "${pkgdatadir}/icon-effect")

############# ILLUSION #################
ILLUSION_PACKAGE=cd-illusion
AC_SUBST(GETTEXT_ILLUSION, "cairo-dock-plugins")

AC_SUBST(VERSION_ILLUSION, "1.0.3")
AC_SUBST(PACKAGE_ILLUSION, "$ILLUSION_PACKAGE")

AC_SUBST(illusiondatadir, "${pkgdatadir}/illusion")

############# KEYBOARD_INDICATOR #################
KEYBOARD_INDICATOR_PACKAGE="cd-keyboard-indicator"
AC_SUBST(GETTEXT_KEYBOARD_INDICATOR, "cairo-dock-plugins")

AC_SUBST(VERSION_KEYBOARD_INDICATOR, "1.1.0")
AC_SUBST(PACKAGE_KEYBOARD_INDICATOR, "$KEYBOARD_INDICATOR_PACKAGE")

AC_SUBST(keyboard_indicatordatadir, "${pkgdatadir}/keyboard-indicator")

############# LOGOUT #################
LOGOUT_PACKAGE=cd-logout
AC_SUBST(GETTEXT_LOGOUT, "$LOGOUT_PACKAGE")

AC_SUBST(VERSION_LOGOUT, "1.2.7")
AC_SUBST(PACKAGE_LOGOUT, "$LOGOUT_PACKAGE")

AC_SUBST(logoutdatadir, "${pkgdatadir}/logout")

############# MAIL #################
MAIL_PACKAGE=cd-mail
AC_SUBST(GETTEXT_MAIL, "cairo-dock-plugins")

AC_SUBST(VERSION_MAIL, "1.0.0-beta")
AC_SUBST(PACKAGE_MAIL, "$MAIL_PACKAGE")

AC_SUBST(maildatadir, "${pkgdatadir}/mail")

############# MOTION BLUR #################
MOTION_BLUR_PACKAGE=cd-motion_blur
AC_SUBST(GETTEXT_MOTION_BLUR, "cairo-dock-plugins")

AC_SUBST(VERSION_MOTION_BLUR, "1.0.3")
AC_SUBST(PACKAGE_MOTION_BLUR, "$MOTION_BLUR_PACKAGE")

AC_SUBST(motion_blurdatadir, "${pkgdatadir}/motion-blur")

############# LOGOUT #################
MUSICPLAYER_PACKAGE=cd-musicplayer
AC_SUBST(GETTEXT_MUSICPLAYER, "$MUSICPLAYER_PACKAGE")

AC_SUBST(VERSION_MUSICPLAYER, "1.0.0-beta")
AC_SUBST(PACKAGE_MUSICPLAYER, "$MUSICPLAYER_PACKAGE")

AC_SUBST(musicplayerdatadir, "${pkgdatadir}/musicPlayer")

############# NETSPEED #################
NETSPEED_PACKAGE=cd-netspeed
AC_SUBST(GETTEXT_NETSPEED, "cairo-dock-plugins")

AC_SUBST(VERSION_NETSPEED, "1.2.1")
AC_SUBST(PACKAGE_NETSPEED, "$NETSPEED_PACKAGE")

AC_SUBST(netspeeddatadir, "${pkgdatadir}/netspeed")

############# POWERMANAGER #################
POWERMANAGER_PACKAGE=cd-powermanager
AC_SUBST(GETTEXT_POWERMANAGER, "cairo-dock-plugins")

AC_SUBST(VERSION_POWERMANAGER, "1.3.0")
AC_SUBST(PACKAGE_POWERMANAGER, "$POWERMANAGER_PACKAGE")

AC_SUBST(powermanagerdatadir, "${pkgdatadir}/powermanager")

############# RENDERING #################
RENDERING_PACKAGE=cd-rendering
AC_SUBST(GETTEXT_RENDERING, "cairo-dock-plugins")

AC_SUBST(VERSION_RENDERING, "1.5.6")
AC_SUBST(PACKAGE_RENDERING, "$RENDERING_PACKAGE")

AC_SUBST(renderingdatadir, "${pkgdatadir}/rendering")

############# DESKLET_RENDERING #################
DESKLET_RENDERING_PACKAGE=cd-desklet-rendering
AC_SUBST(GETTEXT_DESKLET_RENDERING, "cairo-dock-plugins")

AC_SUBST(VERSION_DESKLET_RENDERING, "1.5.4")
AC_SUBST(PACKAGE_DESKLET_RENDERING, "$DESKLET_RENDERING_PACKAGE")

AC_SUBST(desklet_renderingdatadir, "${pkgdatadir}/desklet-rendering")

############# DIALOG_RENDERING #################
DIALOG_RENDERING_PACKAGE=cd-dialog-rendering
AC_SUBST(GETTEXT_DIALOG_RENDERING, "cairo-dock-plugins")

AC_SUBST(VERSION_DIALOG_RENDERING, "0.4.3")
AC_SUBST(PACKAGE_DIALOG_RENDERING, "$DIALOG_RENDERING_PACKAGE")

AC_SUBST(dialog_renderingdatadir, "${pkgdatadir}/dialog-rendering")

############# QUICK BROWSER #################
QUICK_BROWSER_PACKAGE=cd-quick-browser
AC_SUBST(GETTEXT_QUICK_BROWSER, "cairo-dock-plugins")

AC_SUBST(VERSION_QUICK_BROWSER, "1.0.3")
AC_SUBST(PACKAGE_QUICK_BROWSER, "$QUICK_BROWSER_PACKAGE")

AC_SUBST(quick_browserdatadir, "${pkgdatadir}/quick_browser")

############# SHORTCUTS #################
SHORTCUTS_PACKAGE=cd-shortcuts
AC_SUBST(GETTEXT_SHORTCUTS, "cairo-dock-plugins")

AC_SUBST(VERSION_SHORTCUTS, "1.3.0")
AC_SUBST(PACKAGE_SHORTCUTS, "$SHORTCUTS_PACKAGE")

AC_SUBST(shortcutsdatadir, "${pkgdatadir}/shortcuts")

############# SHOW DESKLETS #################
SHOW_DESKLETS_PACKAGE=cd-showDesklets
AC_SUBST(GETTEXT_SHOW_DESKLETS, "cairo-dock-plugins")

AC_SUBST(VERSION_SHOW_DESKLETS, "1.0.5")
AC_SUBST(PACKAGE_SHOW_DESKLETS, "$SHOW_DESKLETS_PACKAGE")

AC_SUBST(show_deskletsdatadir, "${pkgdatadir}/showDesklets")

############# SHOW DESKTOP #################
SHOW_DESKTOP_PACKAGE=cd-showDesktop
AC_SUBST(GETTEXT_SHOW_DESKTOP, "cairo-dock-plugins")

AC_SUBST(VERSION_SHOW_DESKTOP, "1.1.0")
AC_SUBST(PACKAGE_SHOW_DESKTOP, "$SHOW_DESKTOP_PACKAGE")

AC_SUBST(show_desktopdatadir, "${pkgdatadir}/showDesktop")

############# SHOW MOUSE #################
SHOW_MOUSE_PACKAGE=cd-show_mouse
AC_SUBST(GETTEXT_SHOW_MOUSE, "cairo-dock-plugins")

AC_SUBST(VERSION_SHOW_MOUSE, "1.0.3")
AC_SUBST(PACKAGE_SHOW_MOUSE, "$SHOW_MOUSE_PACKAGE")

AC_SUBST(show_mousedatadir, "${pkgdatadir}/show_mouse")

############# SLIDER #################
SLIDER_PACKAGE="cd-slider"
AC_SUBST(GETTEXT_SLIDER, "cairo-dock-plugins")

AC_SUBST(VERSION_SLIDER, "2.0.3")
AC_SUBST(PACKAGE_SLIDER, "$SLIDER_PACKAGE")

AC_SUBST(sliderdatadir, "${pkgdatadir}/slider")

############# STACK #################
STACK_PACKAGE="cd-stack"
AC_SUBST(GETTEXT_STACK, "cairo-dock-plugins")

AC_SUBST(VERSION_STACK, "0.2.5")
AC_SUBST(PACKAGE_STACK, "$STACK_PACKAGE")

AC_SUBST(stackdatadir, "${pkgdatadir}/stack")

############# SWITCHER #################
SWITCHER_PACKAGE="cd-switcher"
AC_SUBST(GETTEXT_SWITCHER, "cairo-dock-plugins")

AC_SUBST(VERSION_SWITCHER, "1.0.7")
AC_SUBST(PACKAGE_SWITCHER, "$SWITCHER_PACKAGE")

AC_SUBST(switcherdatadir, "${pkgdatadir}/switcher")

############# SYSTEM MONITOR #################
SYSTEM_MONITOR_PACKAGE="cd-system-monitor"
AC_SUBST(GETTEXT_SYSTEM_MONITOR, "cairo-dock-plugins")

AC_SUBST(VERSION_SYSTEM_MONITOR, "1.0.0-beta")
AC_SUBST(PACKAGE_SYSTEM_MONITOR, "$SYSTEM_MONITOR_PACKAGE")

AC_SUBST(system_monitordatadir, "${pkgdatadir}/System-monitor")

############# SYSTRAY #################
SYSTRAY_PACKAGE=cd-systray
AC_SUBST(GETTEXT_SYSTRAY, "cairo-dock-plugins")

AC_SUBST(VERSION_SYSTRAY, "0.1.9")
AC_SUBST(PACKAGE_SYSTRAY, "$SYSTRAY_PACKAGE")

AC_SUBST(systraydatadir, "${pkgdatadir}/systray")

############# TERMINAL #################
TERMINAL_PACKAGE="cd-terminal"
AC_SUBST(GETTEXT_TERMINAL, "cairo-dock-plugins")

AC_SUBST(VERSION_TERMINAL, "1.0.6")
AC_SUBST(PACKAGE_TERMINAL, "$TERMINAL_PACKAGE")

AC_SUBST(terminaldatadir, "${pkgdatadir}/terminal")

############# TOMBOY #################
TOMBOY_PACKAGE="cd-tomboy"
AC_SUBST(GETTEXT_TOMBOY, "cairo-dock-plugins")

AC_SUBST(VERSION_TOMBOY, "1.1.0")
AC_SUBST(PACKAGE_TOMBOY, "$TOMBOY_PACKAGE")

AC_SUBST(tomboydatadir, "${pkgdatadir}/tomboy")

############# TOONS #################
TOONS_PACKAGE="cd-Toons"
AC_SUBST(GETTEXT_TOONS, "cairo-dock-plugins")

AC_SUBST(VERSION_TOONS, "1.0.4")
AC_SUBST(PACKAGE_TOONS, "$TOONS_PACKAGE")

AC_SUBST(toonsdatadir, "${pkgdatadir}/Toons")
AC_SUBST(toonsuserdirname,"Toons")

############# WEATHER #################
WEATHER_PACKAGE="cd-weather"
AC_SUBST(GETTEXT_WEATHER, "cairo-dock-plugins")

AC_SUBST(VERSION_WEATHER, "1.2.6")
AC_SUBST(PACKAGE_WEATHER, "$WEATHER_PACKAGE")

AC_SUBST(weatherdatadir, "${pkgdatadir}/weather")
AC_SUBST(weatheruserdirname, "weather")

############# WIFI #################
WIFI_PACKAGE="cd-wifi"
AC_SUBST(GETTEXT_WIFI, "cairo-dock-plugins")

AC_SUBST(VERSION_WIFI, "1.2.1")
AC_SUBST(PACKAGE_WIFI, "$WIFI_PACKAGE")

AC_SUBST(wifidatadir, "${pkgdatadir}/wifi")

############# XFCE-INTEGRATION #################
XFCE_INTEGRATION_PACKAGE=cd_xfce-integration

AC_SUBST(VERSION_XFCE_INTEGRATION, "1.0.2")
AC_SUBST(PACKAGE_XFCE_INTEGRATION, "$XFCE_INTEGRATION_PACKAGE")

AC_SUBST(xfce_integrationdatadir, "${pkgdatadir}/xfce-integration")

############# XGAMMA #################
XGAMMA_PACKAGE="cd-Xgamma"
AC_SUBST(GETTEXT_XGAMMA, "cairo-dock-plugins")

AC_SUBST(VERSION_XGAMMA, "1.1.3")
AC_SUBST(PACKAGE_XGAMMA, "$XGAMMA_PACKAGE")

AC_SUBST(xgammadatadir, "${pkgdatadir}/Xgamma")

############# WEBLETS #################
WEBLETS_PACKAGE="cd-weblets"

AC_DEFINE(HAVE_WEBKIT,1,[Defined if we want to use webkit.])

AC_SUBST(GETTEXT_WEBLETS, "cairo-dock-plugins")

AC_SUBST(VERSION_WEBLETS, "0.3.3")
AC_SUBST(PACKAGE_WEBLETS, "$WEBLETS_PACKAGE")

AC_SUBST(webletsdatadir, "${pkgdatadir}/weblets")


AC_CONFIG_FILES([
	Makefile
	po/Makefile.in
	
	alsaMixer/Makefile
	alsaMixer/src/Makefile
	alsaMixer/data/Makefile
	alsaMixer/data/AlsaMixer.conf
	
	Animated-icons/Makefile
	Animated-icons/src/Makefile
	Animated-icons/data/Makefile
	Animated-icons/data/Animated-icons.conf
	
	Cairo-Penguin/Makefile
	Cairo-Penguin/src/Makefile
	Cairo-Penguin/data/Makefile
	Cairo-Penguin/data/Cairo-Penguin.conf
	Cairo-Penguin/data/themes/Makefile
	Cairo-Penguin/data/themes/Classic/Makefile
	
	Clipper/Makefile
	Clipper/src/Makefile
	Clipper/data/Makefile
	Clipper/data/Clipper.conf
	
	clock/Makefile
	clock/src/Makefile
	clock/data/Makefile
	clock/data/clock.conf
	clock/data/digital/Makefile
	clock/data/digital/default/Makefile
	clock/data/themes/Makefile
	clock/data/themes/default/Makefile
	
	compiz-icon/Makefile
	compiz-icon/src/Makefile
	compiz-icon/data/Makefile
	compiz-icon/data/compiz-icon.conf
	
	Dbus/Makefile
	Dbus/src/Makefile
	Dbus/data/Makefile
	Dbus/data/Dbus.conf
	
	dnd2share/Makefile
	dnd2share/src/Makefile
	dnd2share/data/Makefile
	dnd2share/data/dnd2share.conf
	
	drop-indicator/Makefile
	drop-indicator/src/Makefile
	drop-indicator/data/Makefile
	drop-indicator/data/drop_indicator.conf
	
	dustbin/Makefile
	dustbin/src/Makefile
	dustbin/data/Makefile
	dustbin/data/dustbin.conf
	dustbin/data/themes/Makefile
	dustbin/data/themes/default/Makefile
	
	GMenu/Makefile
	GMenu/src/Makefile
	GMenu/data/Makefile
	GMenu/data/GMenu.conf
	
	gnome-integration/Makefile
	gnome-integration/src/Makefile
	gnome-integration/data/Makefile
	
	gnome-integration-old/Makefile
	gnome-integration-old/src/Makefile
	gnome-integration-old/data/Makefile
	
	icon-effect/Makefile
	icon-effect/src/Makefile
	icon-effect/data/Makefile
	icon-effect/data/icon-effect.conf
	
	illusion/Makefile
	illusion/src/Makefile
	illusion/data/Makefile
	illusion/data/illusion.conf
	
	keyboard-indicator/Makefile
	keyboard-indicator/src/Makefile
	keyboard-indicator/data/Makefile
	keyboard-indicator/data/keyboard-indicator.conf
	
	logout/Makefile
	logout/src/Makefile
	logout/data/Makefile
	logout/data/logout.conf
	
	mail/Makefile
	mail/src/Makefile
	mail/data/Makefile
	mail/data/mail.conf
	mail/data/themes/Makefile
	mail/data/themes/Default/Makefile
	
	motion-blur/Makefile
	motion-blur/src/Makefile
	motion-blur/data/Makefile
	motion-blur/data/motion_blur.conf
	
	musicPlayer/Makefile
	musicPlayer/src/Makefile
	musicPlayer/data/Makefile
	musicPlayer/data/musicPlayer.conf
	musicPlayer/data/themes/Makefile
	musicPlayer/data/themes/cd_box_simple/Makefile
	
	netspeed/Makefile
	netspeed/src/Makefile
	netspeed/data/Makefile
	netspeed/data/netspeed.conf
	
	powermanager/Makefile
	powermanager/src/Makefile
	powermanager/data/Makefile
	powermanager/data/powermanager.conf
	
	dock-rendering/Makefile
	dock-rendering/src/Makefile
	dock-rendering/data/Makefile
	dock-rendering/data/rendering.conf
	
	desklet-rendering/Makefile
	desklet-rendering/src/Makefile
	desklet-rendering/data/Makefile
	
	dialog-rendering/Makefile
	dialog-rendering/src/Makefile
	dialog-rendering/data/Makefile
	dialog-rendering/data/dialog-rendering.conf
	
	quick-browser/Makefile
	quick-browser/src/Makefile
	quick-browser/data/Makefile
	quick-browser/data/quick-browser.conf
	
	shortcuts/Makefile
	shortcuts/src/Makefile
	shortcuts/data/Makefile
	shortcuts/data/shortcuts.conf
	
	showDesklets/Makefile
	showDesklets/src/Makefile
	showDesklets/data/Makefile
	showDesklets/data/showDesklets.conf
	
	showDesktop/Makefile
	showDesktop/src/Makefile
	showDesktop/data/Makefile
	showDesktop/data/showDesktop.conf
	
	show-mouse/Makefile
	show-mouse/src/Makefile
	show-mouse/data/Makefile
	show-mouse/data/show_mouse.conf
	
	slider/Makefile
	slider/src/Makefile
	slider/data/Makefile
	slider/data/slider.conf
	
	stack/Makefile
	stack/src/Makefile
	stack/data/Makefile
	stack/data/stack.conf

	switcher/Makefile
	switcher/src/Makefile
	switcher/data/Makefile
	switcher/data/switcher.conf
	
	System-Monitor/Makefile
	System-Monitor/src/Makefile
	System-Monitor/data/Makefile
	System-Monitor/data/System-Monitor.conf
	
	systray/Makefile
	systray/src/Makefile
	systray/data/Makefile
	systray/data/systray.conf
	
	terminal/Makefile
	terminal/src/Makefile
	terminal/data/Makefile
	terminal/data/terminal.conf
	
	tomboy/Makefile
	tomboy/src/Makefile
	tomboy/data/Makefile
	tomboy/data/tomboy.conf
	
	Toons/Makefile
	Toons/src/Makefile
	Toons/data/Makefile
	Toons/data/Toons.conf
	Toons/data/themes/Makefile
	Toons/data/themes/Classic/Makefile
	
	weather/Makefile
	weather/src/Makefile
	weather/data/Makefile
	weather/data/weather.conf
	weather/data/themes/Makefile
	weather/data/themes/Classic/Makefile
	
	weblets/Makefile
	weblets/src/Makefile
	weblets/data/Makefile
	weblets/data/weblets.conf
	
	wifi/Makefile
	wifi/src/Makefile
	wifi/data/Makefile
	wifi/data/wifi.conf
	
	xfce-integration/Makefile
	xfce-integration/src/Makefile
	xfce-integration/data/Makefile
	
	Xgamma/Makefile
	Xgamma/src/Makefile
	Xgamma/data/Makefile
	Xgamma/data/Xgamma.conf
	])

AC_OUTPUT([po/Makefile
	])

echo "Cairo-Dock-plugins will be compiled with the following options :"
echo " * Compil gnome-integration plug-in : "${with_gnome_support}
echo " * Compil gnome-integration-old plug-in : "${with_old_gnome_support}
echo " * Compil xfce-integration plug-in : "${with_xfce_support}
echo " * Compil AlsaMixer applet : "${with_alsa_mixer}
echo " * Compil Terminal applet : "${with_terminal}
echo " * Compil PowerManager applet : "${with_powermanager}
echo " * Compil Wifi applet : "${with_wifi}
echo " * Compil Tomboy applet : "${with_tomboy}
echo " * Compil Xgamma applet : "${with_xgamma}
echo " * Compil GMenu applet : "${with_gmenu}
echo " * Enable gio in GMenu applet : "${with_gio_in_gmenu}
echo " * Compil Mail applet : "${with_mail}
echo " * Compil Weblets applet : "${with_weblets}
echo " * Compil keyboard-indicator applet : "${with_keyboard_indicator}
echo " * Compil Dnd2share applet : "${with_dnd2share}
echo " * Compil MusicPlayer applet : "${with_musicplayer}


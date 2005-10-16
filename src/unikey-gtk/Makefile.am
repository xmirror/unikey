if PLATFORM_WIN32
no_undefined = -no-undefined
endif

TMP = @unikey_gtk@
moduledir = @GTK_LIBDIR@/gtk-2.0/@GTK_VERSION@/immodules

module_LTLIBRARIES = im-vn.la

im_vn_la_LDFLAGS = -rpath $(moduledir) -avoid-version -module $(no_undefined)

AM_CPPFLAGS = -Wall @STRIP_BEGIN@ \
	-DG_DISABLE_DEPRECATED	\
	-DGDK_DISABLE_DEPRECATED	\
	-DGTK_DISABLE_DEPRECATED 	\
	@GTK_CFLAGS@ \
	-I ../ukinterface -I ../ukengine -I ../vnconv \
@STRIP_END@

im_vn_la_SOURCES = 	\
	gtkimcontextvn.c 	\
	gtkimcontextvn.h	\
	imvn.c \
	uksync.c uksync.h \
	nls.h \
	dummy.cpp \
	../gui/xvnkb.h \
	../xim/optparse.c ../xim/optparse.h \
	../xim/ukopt.c ../xim/ukopt.h
##	../ukengine/macro.cpp ../ukengine/macro.h \
##	../ukengine/mactab.cpp ../ukengine/mactab.h

im_vn_la_LIBADD = @GTK_LIBS@ ../ukinterface/libukint.la

install-data-hook:
	gtk-query-immodules-2.0 \
	> @gtk_sysconfdir@/gtk.immodules

##	$(moduledir)/im-vn.la \
##	@GTK_PREFIX@/lib/gtk-2.0/@GTK_VERSION@/immodules/*.la \


uninstall-hook:
	gtk-query-immodules-2.0 > \
	@gtk_sysconfdir@/gtk.immodules
##	@GTK_PREFIX@/lib/gtk-2.0/@GTK_VERSION@/immodules/*.la \


##gtk.immodules: Makefile.am $(module_LTLIBRARIES)
##	/usr/bin/gtk-query-immodules-2.0 im-vn.la > gtk.immodules
##
##all-local: gtk.immodules

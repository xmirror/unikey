AM_CPPFLAGS = -Wall -I../IMdkit -I../ukengine -I ../vnconv
bin_PROGRAMS = ukxim
ukxim_SOURCES = \
	xim.c IC.c IC.h \
	uksync.c uksync.h ../gui/xvnkb.h \
	macro.cpp macro.h \
	mactab.cpp mactab.h \
	optparse.c optparse.h \
	ukopt.c ukopt.h

ukxim_CFLAGS =  \
    @X_CFLAGS@
##ukxim_LDADD = ../IMdkit/libXimd.a ../ukengine/libUnikey.la
ukxim_LDADD = ../IMdkit/libIMdkit.la ../ukengine/libUnikey.la ../vnconv/libvnconv.la
ukxim_LDFLAGS = @X_LIBS@ @X_PRE_LIBS@ -lX11

EXTRA_DIST = install.sed uninstall.sed

sysbash=/etc/profile
install-data-hook:
	sed -f uninstall.sed $(sysbash) | sed -f install.sed > tmpfile
	mv tmpfile $(sysbash)

##sed "/^export XMODIFIERS/d" /etc/profile > /etc/profile
##echo -e "export XMODIFIERS=\"@im=unikey\"\nexport GTK_IM_MODULE=xim" \
##>> /etc/profile

uninstall-hook:
	sed -f uninstall.sed $(sysbash) > tmpfile && mv tmpfile $(sysbash)

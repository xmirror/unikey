bin_PROGRAMS = unikey
unikey_SOURCES = gui.c uksync.c xvnkb.h uksync.h \
	../xim/optparse.c ../xim/optparse.h \
	guiopt.c guiopt.h

unikey_CFLAGS = -Wall   \
    @X_CFLAGS@
unikey_LDFLAGS = @X_LIBS@ @X_PRE_LIBS@ -lX11

#

all::
clean::;	rm -f *~
install::

PREFIX		?=	/usr/local

CPP		=	g++

#CFLAGS		=	-std=c++11
CFLAGS		=	-std=c++14
#CFLAGS		=	-std=c++17
CFLAGS		+=	-g
#CFLAGS		+=	-Wall -W
CFLAGS		+=	-I.
CFLAGS		+=	-fstack-protector-strong

LDFLAGS		=

# Mosquitto
LDFLAGS		+=	-lmosquitto

BINS		=	scxmlrun
OBJS		=	jsonstream.o scxmlproc.o

%.o:	%.cpp
	$(CPP) $(CFLAGS) -c $< -o $@

all::	$(BINS)

# ================================================================================
ifndef	PROCESSOR
PROCESSOR	=	qtscxml
endif

# -------
# QtScxml
# -------
ifeq	($(PROCESSOR), qtscxml)

OBJS		+=	adapters/qtscxmlproc.o

CFLAGS		+=	-DUSE_QTSCXML

# Qt
QTVERSION	?=	5.9.5
CFLAGS	 	+=	-I/usr/include/x86_64-linux-gnu/qt5

CFLAGS	 	+=	-I/usr/include/x86_64-linux-gnu/qt5/QtCore
CFLAGS	 	+=	-I/usr/include/x86_64-linux-gnu/qt5/QtCore/$(QTVERSION)
CFLAGS	 	+=	-I/usr/include/x86_64-linux-gnu/qt5/QtCore/$(QTVERSION)/QtCore

CFLAGS		+=	-fPIC
CFLAGS		+=	-fdebug-prefix-map=/build/qtbase-opensource-src-oTRRwM/qtbase-opensource-src-$(QTVERSION)+dfsg=.
LDFLAGS		+=	-lQt5Scxml -lQt5Qml -lQt5Core
LDFLAGS		+=	-lpthread

adapters/moc_%.hpp:	adapters/%.hpp
	moc $< -o $@
adapters/moc_%.cpp:	adapters/%.cpp
	moc $< -o $@

endif

# -------
# uScxml
# -------
ifeq	($(PROCESSOR), uscxml)

OBJS		+=	adapters/uscxmlproc.o

# uSCXML
#CPP		:=	g++-5
CPP		:=	g++-6

CFLAGS		+=	-DUSE_USCXML
CFLAGS	 	+=	-I/usr/local/include
LDFLAGS		+=	-luscxml

# WebKit JavaScriptCore
CFLAGS 		+=	-I/usr/include/webkitgtk-4.0
LDFLAGS		+=	-ljavascriptcoregtk-4.0

# XERCES-C
LDFLAGS		+=	-lxerces-c

endif

# ================================================================================

scxmlrun:	$(OBJS) scxmlrun.o
	$(CPP) $^ -o $@ $(LDFLAGS)

-include Makefile.depend
Makefile.depend::
	$(CPP) $(CFLAGS) -MM {.,adapters}/*.[ch]pp > $@

install::	all
	for f in $(BINS); do \
	  cmp -s $$f $(PREFIX)/bin/$$f && continue;\
	  cp -fp $(BINS) $(PREFIX)/bin;\
	done

clean::
	rm -f {.,adapters}/*.o

veryclean::	clean
	rm -f $(BINS)

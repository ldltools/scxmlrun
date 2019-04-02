#

all::

TOP_DIR		=	$(shell readlink -f ..)
BUILD_DIR	=	$(TOP_DIR)/_build/scxmlrun
PREFIX		?=	$(TOP_DIR)/_build/install
SRC_DIRS	=	.
VERSION		= $(shell echo -e '\#include "version.hpp"\nSCXMLRUN_VERSION' | cpp -P | sed 's/\"//g')

BINS		=	scxmlrun
BINS		:=	$(BINS:%=$(BUILD_DIR)/%)

LIBS		=	libscxmlrun.so.$(VERSION)
LIBS		:=	$(LIBS:%=$(BUILD_DIR)/%)

CPP		=	g++
#CPPFLAGS	=	-std=c++11
CPPFLAGS	=	-std=c++14
#CPPFLAGS	=	-std=c++17
CPPFLAGS	+=	-g
#CPPFLAGS	+=	-Wall -W
CPPFLAGS	+=	-I.
CPPFLAGS	+=	-fstack-protector-strong
CPPFLAGS	+=	-fPIC

LDFLAGS		=
LDFLAGS		+=	-lmosquitto

#
#SRCS_hpp	=	jsonstream.hpp scxmlproc.hpp
#SRCS_cpp	=	jsonstream.cpp scxmlproc.cpp

# ================================================================================
# use QtSCXML (default)
# ref: https://doc.qt.io/qt-5/qtscxml-overview.html
# ================================================================================

ifndef	PROCESSOR
PROCESSOR	=	qtscxml
endif

# -------
# QtScxml
# -------
ifeq	($(PROCESSOR), qtscxml)

CPPFLAGS	+=	-DUSE_QTSCXML

# Qt
QTINCDIR	?=	/usr/include/x86_64-linux-gnu/qt5
#QTVERSION	?=	5.9.5
QTVERSION	?=	$(shell gawk '/QTCORE_VERSION_STR/{print(gensub(/\"(.*)\"/,"\\1","g",$$3))}' $(QTINCDIR)/QtCore/qtcoreversion.h)
CPPFLAGS 	+=	-I$(QTINCDIR)

CPPFLAGS	+=	-I$(QTINCDIR)/QtCore
CPPFLAGS	+=	-I$(QTINCDIR)/QtCore/$(QTVERSION)
CPPFLAGS	+=	-I$(QTINCDIR)/QtCore/$(QTVERSION)/QtCore

CPPFLAGS	+=	-fPIC
CPPFLAGS	+=	-fdebug-prefix-map=/build/qtbase-opensource-src-oTRRwM/qtbase-opensource-src-$(QTVERSION)+dfsg=.
LDFLAGS		+=	-lQt5Scxml -lQt5Qml -lQt5Core
LDFLAGS		+=	-lpthread

SRC_DIRS	+=	qtscxml

#SRCS_hpp	+=	qtscxml/qtscxmlproc.hpp
#SRCS_cpp	+=	qtscxml/qtscxmlproc.cpp

$(BUILD_DIR)/qtscxml/qtscxmlproc.o:	qtscxml/moc_qtscxmlproc.hpp
qtscxml/moc_qtscxmlproc.hpp:	qtscxml/qtscxmlproc.hpp
	moc $< -o $@

endif

# ================================================================================
# use uScxml (experimental)
# ref: https://github.com/tklab-tud/uscxml
# ================================================================================

ifeq	($(PROCESSOR), uscxml)

# uSCXML
#CPP		:=	g++-5
CPP		:=	g++-6

CPPFLAGS	+=	-DUSE_USCXML
CPPFLAGS	+=	-I/usr/local/include
LDFLAGS		+=	-luscxml
LDFLAGS		+=	-Wl,-soname,libscxmlrun.so.$(VERSION)

# WebKit JavaScriptCore
CPPFLAGS	+=	-I/usr/include/webkitgtk-4.0
LDFLAGS		+=	-ljavascriptcoregtk-4.0

# XERCES-C
LDFLAGS		+=	-lxerces-c

SRC_DIRS	+=	uscxml

#SRCS_hpp	+=	adapters/uscxmlproc.hpp
#SRCS_cpp	+=	adapters/uscxmlproc.cpp

endif

# ================================================================================

-include Makefile.common

OBJS		:= $(filter-out $(BINS:%=%.o),$(OBJS))

all::	$(LIBS) $(BINS)

$(BUILD_DIR)/libscxmlrun.so.$(VERSION):	$(OBJS)
	$(CPP) $^ -o $@ -shared $(LDFLAGS)

$(BUILD_DIR)/scxmlrun:	$(BUILD_DIR)/scxmlrun.o $(LIBS)
	$(CPP) -o $@ -Wl,-rpath=$(PREFIX)/lib/scxmlrun $< $(LDFLAGS) libscxmlrun.so.$(VERSION)

install::	install_bins
install_bins::	$(BINS)
	@test -d $(PREFIX)/bin -a \( ! -w $(PREFIX)/bin \) && exit 1 || true
	@mkdir -p $(PREFIX)/bin
	for f in $(BINS); do \
	  cmp -s $$f $(PREFIX)/bin/$$f && continue;\
	  cp -fp $$f $(PREFIX)/bin;\
	done
install::	install_libs
install_libs::	$(LIBS)
	@test -d $(PREFIX)/lib -a \( ! -w $(PREFIX)/lib \) && exit 1 || true
	@mkdir -p $(PREFIX)/lib/scxmlrun
	for f in $(LIBS); do \
	  cmp -s $$f $(PREFIX)/lib/scxmlrun/$$f && continue;\
	  cp -fp $$f $(PREFIX)/lib/scxmlrun;\
	done

clean::
	find . -name '*~' | xargs rm -f

veryclean::	clean
	rm -f $(BINS)

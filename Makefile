# $Id: $

PREFIX		?= /usr/local

#SUBDIRS	= src examples tests
SUBDIRS		= examples

all::
	for d in $(SUBDIRS); do make -C $$d $@; done

install::	all
	for d in $(SUBDIRS); do make -C $$d PREFIX=$(PREFIX) $@; done

clean::
	find . -name '#*' -or -name '*~' -or -name '*.log' | xargs rm -f
	for d in $(SUBDIRS); do make -C $$d PREFIX=$(PREFIX) $@; done

veryclean::	clean
	for d in $(SUBDIRS); do make -C $$d PREFIX=$(PREFIX) $@; done
	rm -rf _build/*

#
GITHOME ?= $(HOME)/git/github.com/ldltools/scxmlrun
rsync::	clean
	test -d $(GITHOME) || exit 1
	rsync -avzop --exclude=_build --exclude=.git --exclude=out --exclude=obsolete ./ $(GITHOME)
tar:	veryclean
	(dir=`basename $$PWD`; cd ..; tar cvJf scxmlrun`date +%y%m%d`.tar.xz --exclude=.git --exclude=_build --exclude=RCS --exclude=obsolete $$dir)

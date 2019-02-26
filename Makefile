# $Id: $

PREFIX		?= /usr/local

SUBDIRS	= src examples tests docs

all::
	for d in $(SUBDIRS); do $(MAKE) -C $$d $@; done

install::	all
	for d in $(SUBDIRS); do $(MAKE) -C $$d PREFIX=$(PREFIX) $@; done

clean::
#	find . -name '#*' -or -name '*~' -or -name '*.log' | xargs rm -f
	for d in $(SUBDIRS); do $(MAKE) -C $$d PREFIX=$(PREFIX) $@; done
	rm -f *~

veryclean::	clean
	rm -rf _build/*

#
GITHOME ?= $(HOME)/git/github.com/ldltools/scxmlrun
rsync::	clean
	test -d $(GITHOME) || exit 1
	rsync -avzop --exclude=_build --exclude=.git --exclude=out --exclude=obsolete --exclude=node_modules ./ $(GITHOME)
tar:	veryclean
	(dir=`basename $$PWD`; cd ..; tar cvJf scxmlrun`date +%y%m%d`.tar.xz --exclude=.git --exclude=_build --exclude=RCS --exclude=obsolete $$dir)

# docker
DOCKER_IMAGE	= ldltools/scxmlrun
.PHONY:	$(DOCKER_IMAGE)-dev $(DOCKER_IMAGE)
$(DOCKER_IMAGE)-dev:
	docker images | grep -q "^$@ " && { echo "** $@ exists"; exit 0; } ||\
	docker build --target builder -t $@ .
$(DOCKER_IMAGE):
	docker images | grep -q "^$@ " && { echo "** $@ exists"; exit 0; } ||\
	docker build -t $@ .

docker-build-all:	$(DOCKER_IMAGE)-dev
docker-build:	$(DOCKER_IMAGE)-dev
docker-run:	$(DOCKER_IMAGE)-dev
	docker run -it --rm $<

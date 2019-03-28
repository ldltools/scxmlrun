# $Id: Makefile,v 1.1 2019/03/27 02:33:58 sato Exp sato $

PREFIX		?= /usr/local

SUBDIRS		= src examples tests docs

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

# docker
DOCKER_REPO	= ldltools/scxmlrun
SHELL		:= /bin/bash
VERSION		= $(shell echo -e '\#include "src/version.hpp"\nSCXMLRUN_VERSION' | cpp -P | sed 's/\"//g')

.PHONY:	$(DOCKER_REPO)-dev $(DOCKER_REPO)

$(DOCKER_REPO)-dev:
	docker build --target builder -t $(DOCKER_REPO)-dev .
$(DOCKER_REPO):
	docker build -t $(DOCKER_REPO) .

docker-build:	docker-build-$(DOCKER_REPO)
docker-run:	check-latest-$(DOCKER_REPO)
#	docker run -it --rm $(DOCKER_REPO)
	docker run -d --rm $(DOCKER_REPO) /usr/sbin/mosquitto -c /etc/mosquitto/mosquitto.conf
	container=$$(docker ps -l --format '{{.ID}}');\
	docker exec -it $$container /bin/bash;\
	docker exec $$container pkill mosquitto

#
define GENRULES
docker-build-all::	docker-build-$(1)
docker-build-$(1):	$(1)

docker-tag::	docker-tag-$(1)
docker-tag-$(1):	check-latest-$(1) docker-untag-$(1)
	docker tag $(1):latest $(1):$$(VERSION)
	docker rmi $(1):latest

docker-untag::	docker-untag-$(1)
docker-untag-$(1):	check-latest-$(1)
	@docker images --format "{{.Repository}}:{{.Tag}}" | grep -q "$(1):$$(VERSION)" && docker rmi $(1):$$(VERSION) || true

check-latest-$(1):
	@docker images --format "{{.Repository}}:{{.Tag}}" | grep -q "$(1):latest" || { echo "** image \"$(1):latest\" not found"; exit 1; }
endef
$(foreach repo,$(DOCKER_REPO)-dev $(DOCKER_REPO),$(eval $(call GENRULES,$(repo))))

#
tar:	veryclean
	(dir=`basename $$PWD`; cd ..; tar cvJf scxmlrun`date +%y%m%d`.tar.xz --exclude=.git --exclude=_build --exclude=RCS --exclude=obsolete $$dir)

#
GITHOME ?= $(HOME)/git/github.com/ldltools/scxmlrun
rsync:	clean
	test -d $(GITHOME) || exit 1
	rsync -avzop --exclude=_build --exclude=.git --exclude=out --exclude=obsolete --exclude=node_modules ./ $(GITHOME)

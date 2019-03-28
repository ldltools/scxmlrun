all::
clean::;	rm -f *~
veryclean::	clean
install::

#
TESTS =	hello echo1 echo2 echo3 echo4 echo5

test:	$(TESTS:%=%.sh)
	shelltest tests.conf

gen:	$(TESTS:%=%.sh)

define GENSCRIPT
$(1).sh:	$(1).json
	scxmlrun-all --no-exec $$< -o $$@
endef
$(foreach t,$(TESTS),$(eval $(call GENSCRIPT,$(t))))

# -*-Makefile-*-

all::

TOP_DIR		= $(shell readlink -f ../../..)
BUILD_DIR	= $(TOP_DIR)/_build/caml/tools/runall
PREFIX		?= $(TOP_DIR)/_build/install
#SRC_DIRS	= .

OPAM_PREFIX	= $(shell opam config var prefix)
OCAMLFIND	= $(OPAM_PREFIX)/bin/ocamlfind
CAMLC		= $(OCAMLFIND) ocamlc
CAMLOPT		= $(OCAMLFIND) ocamlopt

ifdef PROF
#CAMLC		:= $(OCAMLFIND) ocamlcp
#CAMLOPT	:= $(OCAMLFIND) ocamloptp
endif

# for ocamlfind
CAMLFIND_FLAGS	=
CAMLFIND_FLAGS	+= -package ppx_deriving.eq
CAMLFIND_FLAGS	+= -package ppx_deriving.ord
CAMLFIND_FLAGS	+= -package ppx_deriving.show
CAMLFIND_FLAGS	+= -package ppx_deriving_yojson
#CAMLFIND_FLAGS	+= -package bigstring
#CAMLFIND_FLAGS	+= -package sexplib
#CAMLFIND_FLAGS	+= -package ppx_sexp_conv

# on our own
CAML_INC_FLAGS	=
CAML_INC_FLAGS	+= -I $(OPAM_PREFIX)/lib/result
CAML_INC_FLAGS	+= -I $(OPAM_PREFIX)/lib/ppx_deriving
CAML_INC_FLAGS	+= -I $(OPAM_PREFIX)/lib/ppx_deriving_yojson
CAML_INC_FLAGS	+= -I $(OPAM_PREFIX)/lib/yojson
CAML_INC_FLAGS	+= -I $(OPAM_PREFIX)/lib/biniou
CAML_INC_FLAGS	+= -I $(OPAM_PREFIX)/lib/easy-format
#CAML_INC_FLAGS	+= -I $(OPAM_PREFIX)/lib/bigstring
#CAML_INC_FLAGS	+= -I $(OPAM_PREFIX)/lib/sexplib
#CAML_INC_FLAGS	+= -I $(OPAM_PREFIX)/lib/ppx_sexp_conv
#CAML_INC_FLAGS	+= -I $(OPAM_PREFIX)/lib/xml-light

CAMLCFLAGS	= $(CAMLFIND_FLAGS) $(CAML_INC_FLAGS)
CAMLOPTFLAGS	= $(CAMLFIND_FLAGS) $(CAML_INC_FLAGS)

CAMLCFLAGS	+= -unsafe-string 
CAMLOPTFLAGS	+= -unsafe-string

ifdef DEBUG
CAMLCFLAGS	+= -g -w +A
CAMLOPTFLAGS	+= -g -w +A
else
CAMLCFLAGS	+= -noassert -w -A
CAMLOPTFLAGS	+= -noassert -w -A
endif

ifdef PROF
#CAMLCFLAGS	+= -p a
CAMLOPTFLAGS	+= -p
endif

CMAS		=
#CMAS		+= nums.cma
CMAS		+= result.cma ppx_deriving_runtime.cma ppx_deriving_yojson_runtime.cma
CMAS		+= easy_format.cma biniou.cma yojson.cma
CMAS		+= unix.cma
#CMAS		+= unix.cma bigarray.cma bigstring.cma sexplib.cma
#CMAS		+= ppx_sexp_conv_expander.cma ppx_sexp_conv.cma
#CMAS		+= xml-light.cma

CMXAS		= $(CMAS:%.cma=%.cmxa)

# what to build
BINS.stem	= runall
BINS.byte	= $(BINS.stem:%=$(BUILD_DIR)/%.byte)
BINS.opt	= $(BINS.stem:%=$(BUILD_DIR)/%.opt)
BINS		= $(BINS.byte) $(BINS.opt)

LIBS.stem	= process scripter
LIBS.byte	= $(LIBS.stem:%=$(BUILD_DIR)/%.cmo)
LIBS.opt	= $(LIBS.stem:%=$(BUILD_DIR)/%.cmx)
LIBS		= $(LIBS.byte) $(LIBS.opt)

-include Makefile.common

all::	$(BINS)

$(BUILD_DIR)/runall.byte:	$(LIBS.byte) $(BUILD_DIR)/runall_main.cmo
	@echo CMOS: $(CMOS)
	$(CAMLC) $(CAMLCFLAGS) -o $@ $(CMAS) $^
$(BUILD_DIR)/runall.opt:	$(LIBS.opt) $(BUILD_DIR)/runall_main.cmx
	$(CAMLOPT) $(CAMLOPTFLAGS) -o $@ $(CMXAS) $^

#
install::	all install_bins install_libs

install_bins::	all
	mkdir -p $(PREFIX)/bin
	rsync -avzop $(BUILD_DIR)/runall.opt $(PREFIX)/bin/scxmlrun-all

install_libs::	all

clean::
veryclean::

ci::
	for d in `find . -type d`; do \
	  test -d $$d/RCS || continue;\
	  (cd $$d; echo *.ml* | xargs ci -m.);\
	done

Makefile.depend::	$(SRCS_mli) $(SRCS_ml) $(SRCS_mll) $(SRCS_mly)
	@n=0; missing=0;\
	while test $$n -lt 10; do \
	 test -s Makefile.depend || { sleep 1s; continue; };\
	 grep -q '^# generated at' Makefile.depend || { sleep 1s; continue; };\
	 line=`tail -1 Makefile.depend | grep '^# generated at'`;\
	 test ."$$line" = . && exit 0;\
	 break;\
	done

$(BUILD_DIR)/ppx:
	$(CAMLOPT) $(CAML_INC_FLAGS) \
	-I $(HOME)/.opam/system/lib/ppx_driver \
	-I $(HOME)/.opam/system/lib/ppx_core \
	-I $(HOME)/.opam/system/lib/ppx_optcomp \
	-predicates ppx_driver -o $@ -linkpkg \
	-package ppx_deriving -package ppx_deriving.show -package ppx_deriving_yojson \
	{ppx_deriving_runtime,ppx_deriving_yojson_runtime}.cmxa \
	{ppx_core,ppx_optcomp,ppx_driver,ppx_driver_runner}.cmxa

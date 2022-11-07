##
## @brief Enduro/X Python3 bindings module
##
## @file Makefile
##
## -----------------------------------------------------------------------------
## Enduro/X Middleware Platform for Distributed Transaction Processing
## Copyright (C) 2009-2016, ATR Baltic, Ltd. All Rights Reserved.
## Copyright (C) 2017-2018, Mavimax, Ltd. All Rights Reserved.
## This software is released under one of the following licenses:
## AGPL or Mavimax's license for commercial use.
## -----------------------------------------------------------------------------
## AGPL license:
## 
## This program is free software; you can redistribute it and/or modify it under
## the terms of the GNU Affero General Public License, version 3 as published
## by the Free Software Foundation;
##
## This program is distributed in the hope that it will be useful, but WITHOUT ANY
## WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
## PARTICULAR PURPOSE. See the GNU Affero General Public License, version 3
## for more details.
##
## You should have received a copy of the GNU Affero General Public License along 
## with this program; if not, write to the Free Software Foundation, Inc., 
## 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
##
## -----------------------------------------------------------------------------
## A commercial use license is available from Mavimax, Ltd
## contact@mavimax.com
## -----------------------------------------------------------------------------
##

A2X := $(shell a2x --version 2> /dev/null)

SOURCE_DOCS :=$(shell ls -1 *.adoc)
DOCS_MAN8  := $(patsubst %.adoc, man8/%.8, $(SOURCE_DOCS))
DOCS_HTML  := $(patsubst %.adoc, html/%.html, $(SOURCE_DOCS))

.DEFAULT_GOAL := all

check_asciidoc: 
	@type a2x >/dev/null 2>&1 || echo "Missing asciidoc - docs will not built"

all: check_asciidoc $(DOCS_MAN8) $(DOCS_HTML)

#
# Do not remove images
#
.SECONDARY: $(GRAPHS)


#
# Build the document
# So translate the % document and use $%
#
man8/%.8: %.adoc
	a2x -Dman8 --format=manpage $<

html/%.html: %.adoc
	a2x -Dhtml --format=xhtml $<
	
clean:
	- rm -f ./man8/*.8
	- rm -f ./html/*.css ./html/*.html

.PHONY: clean all


# vim: set ts=4 sw=4 et smartindent:

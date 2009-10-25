## Makefile.am file for the XML documents translation.
##
## Copyright (C) 2006 The Xfce development team.
##
## Written by Daichi Kawahata <daichi@xfce.org>, 2006.
##
## This file is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.
##
## This file is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
## 02111-1307, USA.

srcdir = @srcdir@
top_srcdir = @top_srcdir@
docdir = $(top_srcdir)/doc

all-local: $(PACKAGE).pot

$(PACKAGE).pot: $(docdir)/en/$(PACKAGE).xml.in
	xml2po -o $(PACKAGE).pot $(docdir)/en/$(PACKAGE).xml.in

update-po: Makefile $(PACKAGE).pot
	@echo "*** Updating PO ***"
	langs=`sed -e '/^#/d' $(srcdir)/LINGUAS`; \
	for lang in $$langs; do \
		xml2po -u $$lang.po $(docdir)/en/$(PACKAGE).xml.in; \
	done

update-xml: Makefile $(PACKAGE).pot
	@echo "*** Updating XML ***"
	langs=`sed -e '/^#/d' $(srcdir)/LINGUAS`; \
	for lang in $$langs; do \
		if test -n $(docdir)/$$lang; then \
			mkdir -p $(docdir)/$$lang; \
		fi; \
		xml2po -p $$lang.po $(docdir)/en/$(PACKAGE).xml.in > $(docdir)/$$lang/$(PACKAGE).xml.in; \
	done

dist-check-xml2po: all

DISTCLEANFILES = $(PACKAGE).pot


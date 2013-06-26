ifeq (,$(wildcard ../config.mak))
$(error "../config.mak is not present, run configure !")
endif
include ../config.mak

top_srcdir = ..

DOMAIN = ushare

# Languages to generate
LANGS = fr de

GMSGFMT = /usr/bin/msgfmt
XGETTEXT = /usr/bin/xgettext
MSGMERGE = msgmerge
MSGMERGE_UPDATE = /usr/bin/msgmerge --update
MSGINIT = msginit
MSGCONV = msgconv
MSGFILTER = msgfilter

EXTRADIST = $(POFILES) $(GMOFILES) \
	POTFILES \
	$(DOMAIN).pot \

GMOFILES = $(addsuffix .gmo,$(LANGS))
POFILES = $(addsuffix .po,$(LANGS))
POTFILES = $(shell for f in `cat POTFILES`; do echo -n "$(top_srcdir)/$$f "; done)


all: stamp-po

stamp-po: $(DOMAIN).pot
	$(MAKE) $(GMOFILES)
	touch $@

.po.gmo:
	$(GMSGFMT) -c --statistics -o $@ $<

.SUFFIXES: .po .gmo


$(POFILES): $(DOMAIN).pot
	$(MSGMERGE_UPDATE) $@ $(DOMAIN).pot;


update-po:
	$(MAKE) $(DOMAIN).pot-update
	$(MAKE) update-gmo

update-gmo: $(GMOFILES)
	@:

.PHONY: update-po update-gmo

# This target rebuilds $(DOMAIN).pot; it is an expensive operation.
# Note that $(DOMAIN).pot is not touched if it doesn't need to be changed.
$(DOMAIN).pot-update: $(POTFILES) POTFILES
	$(XGETTEXT) --default-domain=$(DOMAIN) --directory=$(top_srcdir) \
	  --add-comments=TRANSLATORS:  \
	  --keyword=_ --keyword=N_ \
	  --files-from=POTFILES \
	  --copyright-holder='http://ushare.geexbox.org' \
	  --msgid-bugs-address="ushare@geexbox.org" \
	  -o $(DOMAIN).pot

# This rule has no dependencies: we don't need to update $(DOMAIN).pot at
# every "make" invocation, only create it when it is missing.
# Only "make $(DOMAIN).pot-update" or "make dist" will force an update.
$(DOMAIN).pot:
	$(MAKE) $(DOMAIN).pot-update

clean:
	-$(RM) -f stamp-po

distclean: clean
	-$(RM) -f $(GMOFILES)

install: $(GMOFILES)
	for gmo in $(GMOFILES); do \
	  lang=`echo $$gmo | sed -e 's/\.gmo$$//'`; \
	  $(INSTALL) -d $(localedir)/$$lang/LC_MESSAGES; \
	  $(INSTALL) -m 644 $$lang.gmo $(localedir)/$$lang/LC_MESSAGES/$(DOMAIN).mo; \
	done


dist-all: $(GMOFILES)
	cp $(EXTRADIST) $(SRCS) Makefile $(DIST)

.PHONY: dist-all install clean distclean

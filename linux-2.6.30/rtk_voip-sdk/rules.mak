ifeq "$(VERSION).$(PATCHLEVEL)" "2.4"

ifndef O_TARGET
O_TARGET := built-in.o
endif

include $(DIR_VOIP)/Makefile.lib

include $(TOPDIR)/Rules.make

# We would rather have a list of rules like
#   foo.o: $(foo-objs)
# but that's not so easy, so we rather make all composite objects depend
# on the set of all their parts
$(multi-used-y) : %.o: $(multi-objs-y)
	$(LD) $(EXTRA_LDFLAGS) -r -o $@ $^

%.s: %.S
	$(CPP) $(AFLAGS) $(EXTRA_AFLAGS) $(AFLAGS_$@) $< > $@

%.o: %.S
	$(CC) $(AFLAGS) $(EXTRA_AFLAGS) $(AFLAGS_$@) -c -o $@ $<

endif

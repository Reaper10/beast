# This Source Code Form is licensed MPL-2.0: http://mozilla.org/MPL/2.0
include $(wildcard $>/tests/*.d)
CLEANDIRS += $(wildcard $>/tests/)
tests-rpath.bse ::= ../bse

# == suite1 ==
tests/suite1.sources		::= $(strip	\
	tests/suite1-main.cc			\
	tests/suite1-randomhash.cc		\
)
tests/suite1			::= $>/tests/suite1
ALL_TARGETS			 += $(tests/suite1)
tests/suite1.objects		::= $(sort $(tests/suite1.sources:%.cc=$>/%.o))
$(tests/suite1.objects):	$(bse/libbse.deps) | $>/tests/
$(tests/suite1.objects):	EXTRA_INCLUDES ::= -I$> $(GLIB_CFLAGS)
$(eval $(call LINKER, $(tests/suite1), $(tests/suite1.objects), $(bse/libbse.solinks), -lbse-$(VERSION_MAJOR) $(GLIB_LIBS), $(tests-rpath.bse)) )

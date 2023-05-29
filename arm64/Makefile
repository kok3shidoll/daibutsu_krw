OLDTOOLCHAIN	?= PATH=/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeOld.xctoolchain/usr/bin:$PATH

CLANG	?= clang -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS9.2.sdk
ARCH	?= -arch arm64

INCDIR = iphone-include
MIGDIR = mig

LIBKERN ?= /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include
OSFMK ?= /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include
IOKIT ?= /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/System/Library/Frameworks/IOKit.framework/Headers

MIG_CC	?= mig -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS10.2.sdk
MIG_FLAGS	?= -arch arm64 -DIOKIT -I../$(INCDIR)

C_FLAGS	?= -I./include
LD_LIBS	?= -framework IOKit

.PHONY: all clean

all: mig
	$(OLDTOOLCHAIN) $(CLANG) $(ARCH) $(C_FLAGS) $(LD_LIBS) *.c -o test -Wl,-no_pie -Wl,-pagezero_size,0x4000 -Wl,-image_base,0x100000000 -miphoneos-version-min=8.0
	strip test
	ldid -Sent.xml test

$(INCDIR):
	mkdir $(INCDIR)
	ln -s $(IOKIT) $(INCDIR)/IOKit
	mkdir $(INCDIR)/libkern
	ln -s $(LIBKERN)/libkern/OSTypes.h $(INCDIR)/libkern/OSTypes.h
	mkdir $(INCDIR)/mach
	ln -s $(OSFMK)/mach/clock_types.defs $(INCDIR)/mach/clock_types.defs
	ln -s $(OSFMK)/mach/mach_types.defs $(INCDIR)/mach/mach_types.defs
	ln -s $(OSFMK)/mach/std_types.defs $(INCDIR)/mach/std_types.defs
	mkdir $(INCDIR)/mach/machine
	ln -s $(OSFMK)/mach/machine/machine_types.defs $(INCDIR)/mach/machine/machine_types.defs

$(MIGDIR): | $(INCDIR)
	mkdir $(MIGDIR)
	cd $(MIGDIR) && $(MIG_CC) $(MIG_FLAGS) $(OSFMK)/device/device.defs

clean:
	-$(RM) -r $(INCDIR) $(MIGDIR)
	-$(RM) test

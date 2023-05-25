CLANG	?= clang -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS10.2.sdk
ARCH	?= -arch armv7

C_FLAGS	?= -I./include
LD_LIBS	?= -framework IOKit

.PHONY: all clean

all:
	$(CLANG) $(ARCH) $(C_FLAGS) $(LD_LIBS) *.c *.o -o test
	ldid -Sent.xml test

clean:
	-$(RM) test

TARGET := KextRW
SRC    := src
TESTS  := tests

# Don't use ?= with $(shell ...)
ifndef CXX_FLAGS
CXX_FLAGS := --std=gnu++17 -Wall -O3 -nostdinc -nostdlib -mkernel -DKERNEL -isystem $(shell xcrun --show-sdk-path)/System/Library/Frameworks/Kernel.framework/Headers -Wl,-kext -lcc_kext $(CXXFLAGS)
endif

.PHONY: all install clean tests

all: $(TARGET).kext/Contents/_CodeSignature/CodeResources tests

$(TARGET).kext/Contents/MacOS/$(TARGET): $(SRC)/*.S $(SRC)/*.cpp $(SRC)/*.h | $(TARGET).kext/Contents/MacOS
	$(CXX) -arch arm64e -o $@ $(SRC)/*.S $(SRC)/*.cpp $(CXX_FLAGS)

$(TARGET).kext/Contents/Info.plist: misc/Info.plist | $(TARGET).kext/Contents
	cp -f $^ $@

$(TARGET).kext/Contents/_CodeSignature/CodeResources: $(TARGET).kext/Contents/MacOS/$(TARGET) $(TARGET).kext/Contents/Info.plist
	codesign -s - -f $(TARGET).kext

$(TARGET).kext/Contents $(TARGET).kext/Contents/MacOS:
	mkdir -p $@

install: all
	sudo cp -R $(TARGET).kext /Library/Extensions/

tests:
	$(MAKE) -C $(TESTS)

clean:
	rm -rf $(TARGET).kext
	$(MAKE) -C $(TESTS) clean
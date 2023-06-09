VER_MAJOR=1
VER_MINOR=1
VER_RELEASE=3

BUILD_DIR_RELEASE=.build_release
BUILD_DIR_DEBUG=.build_debug

BUILD_DIR_EMSCRIPTEN=.build_emscripten

BUNDLE_NAME=roadfighter

UNAME=$(shell uname -s)
ifeq ($(UNAME), Darwin)
	BUNDLE_NAME=roadfighter.app
endif

all:
	@echo "Usage:"
	@echo "    make <linux>       - make linux release"
	@echo "    make <macos>       - make macos release"
	@echo "    make <emscripten>  - make emscripten release"
	@echo "    make <clean>       - cleanup directory"

release_build:
	$(shell if [ ! -d $(BUILD_DIR_RELEASE) ]; then mkdir $(BUILD_DIR_RELEASE); fi)
	cd $(BUILD_DIR_RELEASE) ; cmake -DCMAKE_BUILD_TYPE=Release -DAPP_VERSION_MAJOR:STRING=$(VER_MAJOR) -DAPP_VERSION_MINOR:STRING=$(VER_MINOR) -DAPP_VERSION_RELEASE:STRING=$(VER_RELEASE) .. ; make ; cd ..
	cp -r $(BUILD_DIR_RELEASE)/$(BUNDLE_NAME) .

debug_build:
	$(shell if [ ! -d $(BUILD_DIR_DEBUG) ]; then mkdir $(BUILD_DIR_DEBUG); fi)
	cd $(BUILD_DIR_DEBUG) ; cmake -DCMAKE_BUILD_TYPE=Debug -DAPP_VERSION_MAJOR:STRING=$(VER_MAJOR) -DAPP_VERSION_MINOR:STRING=$(VER_MINOR) -DAPP_VERSION_RELEASE:STRING=$(VER_RELEASE) .. ; make ; cd ..
	cp -r $(BUILD_DIR_DEBUG)/$(BUNDLE_NAME) .

linux: release_build

linux_debug: debug_build

macos: release_build
	cp -r assets $(BUNDLE_NAME)/Contents/Resources/

macos_debug: debug_build
	cp -r assets $(BUNDLE_NAME)/Contents/Resources/

emscripten:
	$(shell if [ ! -d $(BUILD_DIR_EMSCRIPTEN) ]; then mkdir $(BUILD_DIR_EMSCRIPTEN); fi)
	cd $(BUILD_DIR_EMSCRIPTEN) \
		&& emcmake cmake -DCMAKE_BUILD_TYPE=Release \
		-DAPP_VERSION_MAJOR:STRING=$(VER_MAJOR) -DAPP_VERSION_MINOR:STRING=$(VER_MINOR) -DAPP_VERSION_RELEASE:STRING=$(VER_RELEASE) .. \
		&& emmake make
	rm -fr html && mkdir html \
		&& cp $(BUILD_DIR_EMSCRIPTEN)/roadfighter.* html/ \
		&& mv html/roadfighter.html html/index.html

clean:
	rm -fr $(BUILD_DIR_RELEASE) $(BUILD_DIR_DEBUG) $(BUILD_DIR_EMSCRIPTEN) $(BUNDLE_NAME) html

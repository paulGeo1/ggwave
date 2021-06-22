default: build-submodule

.PHONY:
update-submodule:
	# create a clean (maybe updated) copy of ggwave
	rsync ../../include Sources/ggwave/
	rsync ../../include/ggwave/* Sources/ggwave/include/ggwave/
	rsync ../../src/ggwave.cpp Sources/ggwave/
	rsync ../../src/resampler.h Sources/ggwave/
	rsync ../../src/resampler.cpp Sources/ggwave/
	rsync ../../src/reed-solomon Sources/ggwave/
	rsync ../../src/reed-solomon/* Sources/ggwave/reed-solomon/

SOURCES := $(shell find Sources/ -print)
.build: $(SOURCES)
	swift build

.PHONY:
build-submodule: update-submodule Package.swift .build
	touch publish-trigger

.PHONY:
build: Package.swift .build

.PHONY:
publish: publish-trigger
	echo " \
		\n\
		cd /path/to/ggwave/bindings/ios\n\
		git commit\n\
		git tag 0.2.0\n\
		git push origin master --tags\n\
		"

clean:
	rm -rf .build

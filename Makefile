ROOTDIR=$(shell pwd)

SRCDIR=$(ROOTDIR)/src
BUILDDIR=$(ROOTDIR)/build
PREFIX=/zap

CMAKE_BUILD_OPTS=
MAKE_OPTS=
NPROCS=

UNAME_S:=$(shell uname -s)

ifeq ($(UNAME_S),Linux)
	NPROCS=$(shell nproc)
else ifeq ($(UNAME_S),Darwin)
	NPROCS=$(shell sysctl -n hw.ncpu)
endif

ifneq ($(NPROCS),)
	CMAKE_BUILD_OPTS=--parallel $(NPROCS)
	MAKE_OPTS=-j$(NPROCS)
endif

all: check_configured
	@echo "usage: make debug|release"

debug:
	cmake --build $(BUILDDIR)/debug $(CMAKE_BUILD_OPTS)

release: check_configured
	cmake --build $(BUILDDIR)/release $(CMAKE_BUILD_OPTS)

check_configured:
	@if [ ! -d $(BUILDDIR) ]; then \
		echo "please configure first"; \
		exit 1; \
	fi

externals: deps.txt
	./utils/build-externals deps.txt
	touch deps.txt

autocmake:
	@./utils/bootstrap

configure: autocmake
	@for TYPE in debug release; do \
	    rm -rf $(BUILDDIR)/$$TYPE; \
	    cmake \
	        -DCMAKE_TOOLCHAIN_FILE=$(ROOTDIR)/toolchains/$$TYPE.cmake \
	        -DCMAKE_INSTALL_PREFIX=$(PREFIX) \
	        -S $(SRCDIR) \
	        -B $(BUILDDIR)/$$TYPE; \
	done

reconfigure: externals configure

clean:
	@$(MAKE) -C $(BUILDDIR) clean

distclean:
	@rm -rf build ext

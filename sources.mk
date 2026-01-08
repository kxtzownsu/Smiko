REPO_ROOT=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))

# LibSTDC++
LIBCPP  = -lstdc++

# LibUSB
LIBUSB  = $(shell $(PKG_CONFIG) libusb-1.0 --cflags)
ifeq ($(STATIC),)
LIBUSB += $(shell $(PKG_CONFIG) libusb-1.0 --libs)
else
LIBUSB += $(REPO_ROOT)/lib/$(ARCH)/libusb-1.0.a
endif

# LibFTDI
LIBFTDI  = $(shell $(PKG_CONFIG) libftdi1 --cflags)
ifeq ($(STATIC),)
LIBFTDI += $(shell $(PKG_CONFIG) libftdi1 --libs)
else
LIBFTDI += $(REPO_ROOT)/lib/$(ARCH)/libftdi1.a
endif

# LibCrypto
LIBCRYPTO  = $(shell $(PKG_CONFIG) libcrypto --cflags)
ifeq ($(STATIC),)
LIBCRYPTO += $(shell $(PKG_CONFIG) libcrypto --libs)
else
LIBCRYPTO += $(REPO_ROOT)/lib/$(ARCH)/libcrypto.a
CPPFLAGS += -I$(REPO_ROOT)/lib/$(ARCH)/openssl/include
endif

# LibProtobuf
LIBPROTOBUF  = $(shell $(PKG_CONFIG) protobuf-lite --cflags)
LIBPROTOBUF += $(shell $(PKG_CONFIG) protobuf-lite --libs)
ifneq ($(STATIC),)
LIBPROTOBUF += $(REPO_ROOT)/lib/$(ARCH)/libprotobuf-lite.a
endif

# LibTss2
LIBTSS2  = $(shell $(PKG_CONFIG) tss2-sys --cflags)
LIBTSS2 += $(shell $(PKG_CONFIG) tss2-mu --cflags)
LIBTSS2 += $(shell $(PKG_CONFIG) tss2-rc --cflags)
ifeq ($(STATIC),)
LIBTSS2 += $(shell $(PKG_CONFIG) tss2-sys --libs)
LIBTSS2 += $(shell $(PKG_CONFIG) tss2-mu --libs)
LIBTSS2 += $(shell $(PKG_CONFIG) tss2-rc --libs)
else
LIBTSS2 += $(REPO_ROOT)/lib/$(ARCH)/libtss2-sys.a
LIBTSS2 += $(REPO_ROOT)/lib/$(ARCH)/libtss2-mu.a
LIBTSS2 += $(REPO_ROOT)/lib/$(ARCH)/libtss2-rc.a
endif


# Static compilation handler
ifneq ($(STATIC),)
CFLAGS += -static
CXXFLAGS += -static
LDFLAGS += -static
endif

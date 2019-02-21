#
#	Makefile -- Vendor specific build instructions
#

.EXPORT_ALL_VARIABLES:
.PHONY: all romfs image clean

#
# Include architecture specific build rules.
#

ifndef ROOTDIR
ROOTDIR=..
endif

UCLINUX_BUILD_USER=1
-include $(LINUX_CONFIG)
-include $(CONFIG_CONFIG)
-include $(ARCH_CONFIG)
-include $(MODULES_CONFIG)

VEND=$(ROOTDIR)/vendors

dir_v := $(VEND)/$(CONFIG_VENDOR)/$(CONFIG_PRODUCT)/.

# Note: the all and clean targets are not normally used,
# since user/Makefile jumps directory to the product directory
all image clean romfs romfs.post vendor_%:
	$(MAKE) -C $(dir_v) dir_v=$(dir_v) -f $(VEND)/vendors-common.mak $@

prune:
	@for i in `ls -d */ | grep -v -e '$(CONFIG_VENDOR)' -e 'config' -e 'Generic'`; do \
		rm -fr $$i; \
	done
	@for i in `ls -d $(CONFIG_VENDOR)/* | grep -v $(CONFIG_PRODUCT)`; do \
		rm -fr $$i; \
	done


content: vmm.config linux_kernel linux_dtb linux_initrd

vmm.config:
	cp $(REP_DIR)/recipes/raw/vmm_arm64/$@ $@

linux_kernel:
	wget --quiet -O $@.gz http://genode.org/files/release-21.02/linux-kernel-virt-5.11.gz
	gunzip $@.gz

linux_initrd:
	wget --quiet -O $@ http://genode.org/files/release-20.02/initrd-arm64

linux_dtb:
	dtc $(REP_DIR)/src/server/vmm/spec/arm_v8/virt.dts > $@

#block.img:
#	dd if=/dev/zero of=$@ bs=1M count=0 seek=64
#	mkfs.vfat $@

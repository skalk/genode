content: vmm.config linux_kernel linux_dtb block.vdi

vmm.config:
	cp $(REP_DIR)/recipes/raw/vmm_arm64/$@ $@

linux_kernel:
	wget -c -O $@.gz http://genode.org/files/release-21.02/linux-kernel-virt-5.11.gz
	gunzip $@.gz

linux_dtb:
	dtc $(REP_DIR)/src/server/vmm/spec/arm_v8/virt.dts > $@

block.vdi:
	wget -c -O $@.gz http://genode.org/files/release-21.02/buildrootfs.vdi.gz
	gunzip $@.gz

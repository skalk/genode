content: dtb initrd linux vmm.config

linux:
	wget -c -O $@ http://genode.org/files/release-19.11/linux-arm64-image-5.2

initrd:
	wget -c -O $@ http://genode.org/files/release-19.11/initrd-arm64-rcS

vmm.config dtb:
	cp $(REP_DIR)/recipes/raw/vmm-hw/$@ $@

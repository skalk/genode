content: dtb initrd linux vmm.config

linux:
	wget -c -O $@ http://genode.org/files/release-19.11/linux-arm64-fosdem

initrd:
	wget -c -O $@ http://genode.org/files/release-19.11/initrd-arm64-fosdem

vmm.config dtb:
	cp $(REP_DIR)/recipes/raw/vmm-hw/$@ $@

content: vmm.config linux initrd

vmm.config:
	cp $(REP_DIR)/recipes/raw/vmm/$@ $@

initrd:
	wget --quiet -c -O $@ http://http.us.debian.org/debian/dists/bullseye/main/installer-arm64/current/images/netboot/debian-installer/arm64/initrd.gz

linux:
	wget --quiet -c -O $@ http://http.us.debian.org/debian/dists/bullseye/main/installer-arm64/current/images/netboot/debian-installer/arm64/linux

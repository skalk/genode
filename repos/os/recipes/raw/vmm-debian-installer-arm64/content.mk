content: runtime.config vmm.config linux initrd

runtime.config vmm.config:
	cp $(REP_DIR)/recipes/raw/vmm-debian-installer-arm64/$@ $@

initrd:
	wget --quiet -c -O $@ http://http.us.debian.org/debian/dists/bullseye/main/installer-arm64/current/images/netboot/debian-installer/arm64/initrd.gz

linux:
	wget --quiet -c -O $@ http://http.us.debian.org/debian/dists/bullseye/main/installer-arm64/current/images/netboot/debian-installer/arm64/linux

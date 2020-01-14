content: drivers.config fb_drv.config input_filter.config en_us.chargen \
         special.chargen numlock_remap.config

drivers.config input_filter.config:
	cp $(REP_DIR)/recipes/raw/drivers_managed-imx8q_evk/$@ $@

fb_drv.config:
	cp $(GENODE_DIR)/repos/dde_linux/recipes/raw/drivers_interactive-imx8q_evk/$@ $@

numlock_remap.config:
	cp $(REP_DIR)/recipes/raw/drivers_managed-pc/$@ $@

en_us.chargen special.chargen:
	cp $(GENODE_DIR)/repos/os/src/server/input_filter/$@ $@

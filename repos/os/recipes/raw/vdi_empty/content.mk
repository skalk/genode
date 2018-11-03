content: block.vdi block_ext2.vdi block_ext4.vdi uuid.txt

%.vdi:
	cp $(REP_DIR)/recipes/raw/vdi_empty/$@ $@

uuid.txt:
	cp $(REP_DIR)/recipes/raw/vdi_empty/$@ $@

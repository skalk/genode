content: block.vdi uuid.txt

%.vdi:
	cp $(REP_DIR)/recipes/raw/vdi_empty/$@ $@

uuid.txt:
	cp $(REP_DIR)/recipes/raw/vdi_empty/$@ $@

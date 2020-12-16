SRC_DIR := src/test/mobile_shell
include $(GENODE_DIR)/repos/base/recipes/src/content.inc

content: include/decorator

include/decorator:
	mkdir -p $@
	cp $(GENODE_DIR)/repos/os/include/decorator/* $@

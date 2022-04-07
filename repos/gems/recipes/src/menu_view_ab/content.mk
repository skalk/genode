SRC_DIR := src/app/menu_view_ab
include $(GENODE_DIR)/repos/base/recipes/src/content.inc

content: include/polygon_gfx menu_view_base

include/polygon_gfx:
	mkdir -p $@
	cp $(GENODE_DIR)/repos/gems/include/polygon_gfx/* $@

menu_view_base:
	mkdir -p src/app/menu_view
	cp -r $(GENODE_DIR)/repos/gems/src/app/menu_view/styles src/app/menu_view/styles
	cp    $(GENODE_DIR)/repos/gems/src/app/menu_view/*.h src/app/menu_view/. 
	rm src/app/menu_view/label_widget.h

content: src/lib/file lib/mk/file.mk

src/lib/file lib/mk/file.mk:
	$(mirror_from_rep_dir)

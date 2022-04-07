TARGET   = menu_view_ab
SRC_CC   = main.cc
LIBS     = base libc libm vfs libpng zlib blit file
PRG_MENU_DIR = $(PRG_DIR)/../menu_view
INC_DIR += $(PRG_MENU_DIR)

CC_OLEVEL := -O3

CUSTOM_TARGET_DEPS += menu_view_styles.tar

BUILD_ARTIFACTS := $(TARGET) menu_view_styles.tar

.PHONY: menu_view_styles.tar

menu_view_styles.tar:
	$(MSG_CONVERT)$@
	$(VERBOSE)tar cf $@ -C $(PRG_MENU_DIR) styles
	$(VERBOSE)ln -sf $(BUILD_BASE_DIR)/$(PRG_REL_DIR)/$@ $(INSTALL_DIR)/$@

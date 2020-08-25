TARGET   = rpi_new_platform_drv
REQUIRES = arm_v6
SRC_CC   = rpi_device.cc
INC_DIR  = $(PRG_DIR)

vpath %.cc $(PRG_DIR)

include $(REP_DIR)/src/drivers/platform/target.inc

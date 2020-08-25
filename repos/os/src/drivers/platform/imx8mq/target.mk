TARGET   = imx8mq_platform_drv
REQUIRES = arm_v8
SRC_CC   = ccm.cc
SRC_CC  += imx_device.cc
INC_DIR  = $(PRG_DIR)

vpath %.cc $(PRG_DIR)

include $(REP_DIR)/src/drivers/platform/target.inc

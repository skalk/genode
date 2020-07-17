TARGET   = imx8mq_platform_drv
REQUIRES = arm_v8
SRC_CC   = ccm.cc device.cc device_component.cc device_model_policy.cc main.cc session_component.cc root.cc
INC_DIR  = $(PRG_DIR) $(REP_DIR)/src/drivers/platform/spec/arm
LIBS     = base

vpath %.cc $(REP_DIR)/src/drivers/platform/spec/arm

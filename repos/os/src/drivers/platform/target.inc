SRC_CC  += device.cc device_component.cc device_model_policy.cc main.cc session_component.cc root.cc
INC_DIR += $(REP_DIR)/src/drivers/platform
LIBS     = base

vpath %.cc $(REP_DIR)/src/drivers/platform


SPECS    += arm_v7
CC_MARCH ?= -march=armv7-a -mfpu=neon -mfloat-abi=hard

include $(BASE_DIR)/mk/spec/arm_v7.mk


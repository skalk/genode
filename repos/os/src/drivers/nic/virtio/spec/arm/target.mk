TARGET     = virtio_mmio_nic
REQUIRES   = arm
SRC_CC     = mmio_device.cc
LIBS       = base
INC_DIR    = $(REP_DIR)/src/drivers/nic/virtio
CONFIG_XSD = ../../config.xsd

vpath % $(REP_DIR)/src/drivers/nic/virtio

REQUIRES = x86 pci
TARGET   = audio_drv
SRC_CC   = main.cc
LIBS     = dde_bsd_audio base config server
INC_DIR += $(REP_DIR)/include

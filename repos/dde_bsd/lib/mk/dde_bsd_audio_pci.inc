include $(REP_DIR)/lib/mk/dde_bsd_audio.inc

SRC_C  += bsd_emul_pci.c
SRC_CC += pci.cc

# enable when debugging
#CC_OPT += -DAC97_DEBUG
#CC_OPT += -DAUICH_DEBUG
#CC_OPT += -DAZALIA_DEBUG
#CC_OPT += -DDIAGNOSTIC

# AC97 codec
SRC_C += dev/ic/ac97.c

# HDA driver
SRC_C += dev/pci/azalia.c dev/pci/azalia_codec.c

# ICH driver
SRC_C += dev/pci/auich.c

# ES1370
SRC_C += dev/pci/eap.c

# vi: set ft=make :

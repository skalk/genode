TARGET := file_vault

SRC_CC += main.cc

INC_DIR += $(PRG_DIR)
INC_DIR += $(call select_from_repositories,/src/lib/tresor/include)

LIBS += base sandbox vfs

CC_OPT += -Os

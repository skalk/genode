TARGET   = system_manager
SRC_CC   = main.cc runtime.cc
LIBS     = base

# add sculpt_manager to include path to take its config file generation utils
INC_DIR += $(PRG_DIR) $(call select_from_repositories,src/app/sculpt_manager)

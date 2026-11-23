
ALL_EXT_GROUPS += replay
replay_OPT := -DMODULE_NAME=\"replay\"
replay_SRC += ${PX4LIBPATH}/modules/replay/Replay.cpp
replay_SRC += ${PX4LIBPATH}/modules/replay/ReplayEkf2.cpp
replay_SRC += ${PX4LIBPATH}/modules/replay/replay_main.cpp
CPPSOURCES += ${replay_SRC}

ifeq (${CONFIG_MODULE_NSHLIB},y)
MODULE_LIBRARIES += replay_module
MOD_MAIN_replay_module  := replay
MOD_PRIO_replay_module  := 90
MOD_STACK_replay_module := 4096
endif

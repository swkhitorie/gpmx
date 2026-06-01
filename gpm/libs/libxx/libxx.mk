ifeq (${MK_USE_KERNEL_CPP},y)
PROJ_CDEFS += CONFIG_MODULE_KCPP
PROJ_CINCDIRS += ${GPMPATH}/libs/libxx
CPPSOURCES += ${GPMPATH}/libs/libxx/libxx_delete.cpp
CPPSOURCES += ${GPMPATH}/libs/libxx/libxx_delete_sized.cpp
CPPSOURCES += ${GPMPATH}/libs/libxx/libxx_deletea.cpp
CPPSOURCES += ${GPMPATH}/libs/libxx/libxx_deletea_sized.cpp
CPPSOURCES += ${GPMPATH}/libs/libxx/libxx_new.cpp
CPPSOURCES += ${GPMPATH}/libs/libxx/libxx_newa.cpp
endif


ifeq (${MK_USE_LIB_CPP},y)
PROJ_CINCDIRS   +=  libs/libxx
CPPSOURCES +=  libs/libxx/libxx_delete.cpp
CPPSOURCES +=  libs/libxx/libxx_delete_sized.cpp
CPPSOURCES +=  libs/libxx/libxx_deletea.cpp
CPPSOURCES +=  libs/libxx/libxx_deletea_sized.cpp
CPPSOURCES +=  libs/libxx/libxx_new.cpp
CPPSOURCES +=  libs/libxx/libxx_newa.cpp
endif # end with MK_USE_LIB_CPP

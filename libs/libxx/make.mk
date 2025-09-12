
ifeq (${MK_USE_LIB_CPP},y)
PROJ_CINCDIRS   +=  libs/libcpp
CPPSOURCES +=  libs/libcpp/libxx_delete.cpp
CPPSOURCES +=  libs/libcpp/libxx_delete_sized.cpp
CPPSOURCES +=  libs/libcpp/libxx_deletea.cpp
CPPSOURCES +=  libs/libcpp/libxx_deletea_sized.cpp
CPPSOURCES +=  libs/libcpp/libxx_new.cpp
CPPSOURCES +=  libs/libcpp/libxx_newa.cpp
endif # end with MK_USE_LIB_CPP

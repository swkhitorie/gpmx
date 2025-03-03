

ifeq (${CONFIG_FR_MEM_METHOD},1)
FR_CSOURCES += mm/heap_1.c
else ifeq (${CONFIG_FR_MEM_METHOD},2)
FR_CSOURCES += mm/heap_2.c
else ifeq (${CONFIG_FR_MEM_METHOD},3)
FR_CSOURCES += mm/heap_3.c
else ifeq (${CONFIG_FR_MEM_METHOD},4)
FR_CSOURCES += mm/heap_4.c
else ifeq (${CONFIG_FR_MEM_METHOD},5)
FR_CSOURCES += mm/heap_5.c
endif

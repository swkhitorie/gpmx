

ifeq (${MK_USE_RTKMEM_METHOD},y)
CSOURCES += mm/heap_rtx.c
endif

ifeq (${MK_USE_FREERTOS},y)

ifeq (${MK_USE_FRMEM_METHOD},1)
CSOURCES += mm/heap_1.c
else ifeq (${MK_USE_FRMEM_METHOD},2)
CSOURCES += mm/heap_2.c
else ifeq (${MK_USE_FRMEM_METHOD},3)
CSOURCES += mm/heap_3.c
else ifeq (${MK_USE_FRMEM_METHOD},4)
CSOURCES += mm/heap_4.c
else ifeq (${MK_USE_FRMEM_METHOD},5)
CSOURCES += mm/heap_5.c
endif

ifeq (${MK_USE_STLMEM_METHOD},y)
PROJ_CINCDIRS += mm/
endif

endif # end with MK_USE_FREERTOS



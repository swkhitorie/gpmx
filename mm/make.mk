

ifeq (${CONFIG_MK_USE_RTKMEM_METHOD},y)
FR_CSOURCES += mm/heap_rtx.c
endif

ifeq (${CONFIG_MK_USE_FREERTOS},y)

ifeq (${CONFIG_MK_USE_FRMEM_METHOD},1)
FR_CSOURCES += mm/heap_1.c
else ifeq (${CONFIG_MK_USE_FRMEM_METHOD},2)
FR_CSOURCES += mm/heap_2.c
else ifeq (${CONFIG_MK_USE_FRMEM_METHOD},3)
FR_CSOURCES += mm/heap_3.c
else ifeq (${CONFIG_MK_USE_FRMEM_METHOD},4)
FR_CSOURCES += mm/heap_4.c
else ifeq (${CONFIG_MK_USE_FRMEM_METHOD},5)
FR_CSOURCES += mm/heap_5.c
endif

ifeq (${CONFIG_MK_USE_STLMEM_METHOD},y)
FR_CINCDIRS += mm/
endif

endif # end with CONFIG_MK_USE_FREERTOS



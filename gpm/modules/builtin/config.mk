
ifeq (${CONFIG_MODULE_BUILTIN},y)

ifneq (${CONFIG_POSIXRUN_ENABLE},y)
ifneq (${CONFIG_LIBC_PTHREAD},y)
$(error CONFIG_MODULE_BUILTIN depend on CONFIG_LIBC_PTHREAD)
endif
endif

PROJ_CINCDIRS += ${GPMPATH}/modules/builtin
CSOURCES += ${GPMPATH}/modules/builtin/builtin_list.c
CSOURCES += ${GPMPATH}/modules/builtin/builtin_exec.c
CSOURCES += ${GPMPATH}/modules/builtin/builtin_lib.c
CSOURCES += ${GPMPATH}/modules/builtin/builtin_app.c

MODULE_BUILTIN_PATH   := ${GPMPATHABS}/modules/builtin
MODULE_BUILTIN_LIST   := ${MODULE_BUILTIN_PATH}/builtin_list.h
MODULE_BUILTIN_PROTO  := ${MODULE_BUILTIN_PATH}/builtin_proto.h

#-------------------------------------------------------------------------------
# generate_builtin_module
#   MODULE_LIBRARIES       – builtin app name list
#   MOD_MAIN_<module>      – builtin app task main
#   MOD_PRIO_<module>      – builtin app task priority
#   MOD_STACK_<module>     – builtin app task stack size
#-------------------------------------------------------------------------------
define generate_builtin_module
	$(eval _out_dir := $(if $(1),$(1),.))
	$(file > $(_out_dir)/builtin_list.h)
	$(file > $(_out_dir)/builtin_proto.h)
	$(foreach mod, $(sort $(MODULE_LIBRARIES)),
		$(eval _main := $(MOD_MAIN_$(mod)))
		$(if $(_main),
			$(eval _prio := $(if $(MOD_PRIO_$(mod)),$(MOD_PRIO_$(mod)),16))
			$(eval _stack := $(if $(MOD_STACK_$(mod)),$(MOD_STACK_$(mod)),2048))
			$(file >> $(_out_dir)/builtin_list.h,{ "$(_main)", $(_prio), $(_stack) * (__SIZEOF_POINTER__ >> 2), $(_main)_main },)
			$(file >> $(_out_dir)/builtin_proto.h,int $(_main)_main(int argc, char *argv[]);)
		)
	)
endef

.PHONY: ${MODULE_BUILTIN_LIST}

${MODULE_BUILTIN_LIST}:
	$(call generate_builtin_module, ${MODULE_BUILTIN_PATH})

USER_FILE_PREBUILD += ${MODULE_BUILTIN_LIST}

endif

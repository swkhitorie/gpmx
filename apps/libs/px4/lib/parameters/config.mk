
ifeq (${PYTHON_EXECUTABLE},)
PYTHON_EXECUTABLE := python3 
endif
PARAMETERS_DIR        := ${SDK_ROOTDIR}/${PX4LIBPATH}/lib/parameters
PARAMETERS_TARGET_DIR := ${SDK_ROOTDIR}/build/${PROJ_NAME}/parameters

PARAMETERS_PYDEPS := \
    $(PARAMETERS_DIR)/px_process_params.py \
    $(PARAMETERS_DIR)/px4params/srcparser.py \
    $(PARAMETERS_DIR)/px4params/srcscanner.py \
    $(PARAMETERS_DIR)/px4params/jsonout.py \
    $(PARAMETERS_DIR)/px4params/xmlout.py \
    $(PARAMETERS_DIR)/parameters_injected.xml

PARAMETERS_GENDIR      := $(PARAMETERS_TARGET_DIR)/generated_params
PARAMETERS_GEN_XML     := $(PARAMETERS_TARGET_DIR)/parameters.xml
PARAMETERS_GEN_JSON    := $(PARAMETERS_TARGET_DIR)/parameters.json
PARAMETERS_GEN_JSON_XZ := $(PARAMETERS_GEN_JSON).xz
PARAMETERS_HPP         := $(PARAMETERS_TARGET_DIR)/parameters/px4_parameters.hpp

PARAMETERS_MODULES_FILE:= $(PARAMETERS_GENDIR)/module_params.c

PARAMETERS_STAMP       := $(PARAMETERS_TARGET_DIR)/.parameters.stamp

PARAMETERS_OVERRIDES ?= {}
PARAMETERS_BOARD     ?= default

PARAMETERS_MODULE_LIST += \
    ${SDK_ROOTDIR}/${PX4LIBPATH}/lib/controllib/test \
    ${SDK_ROOTDIR}/${PX4LIBPATH}/lib/collision_prevention \
    ${SDK_ROOTDIR}/${PX4LIBPATH}/lib/systemlib

PARAMETERS_MODULE_CFGFILES +=

PARAMETERS_SRC_FILES := $(shell find ${SDK_ROOTDIR}/${PX4LIBPATH} -type f \
    \( -name '*params.c' -o -name '*parameters.c' \) 2>/dev/null)

$(PARAMETERS_GENDIR):
	$(call MK_MKDIR,"$(call MK_PATHTOWIN,$@)")

# define PARAM_GEN_CMD
# @$(PYTHON_EXECUTABLE) $(PARAMETERS_DIR)/px_process_params.py \
#     --src-path $(PARAMETERS_MODULE_LIST) $(PARAMETERS_GENDIR) \
#     --xml $(PARAMETERS_GEN_XML) \
#     --json $(PARAMETERS_GEN_JSON) \
#     --compress \
#     --inject-xml $(PARAMETERS_DIR)/parameters_injected.xml \
#     --overrides '$(PARAMETERS_OVERRIDES)' \
#     --board $(PARAMETERS_BOARD)
# endef

# $(PARAMETERS_STAMP): $(PARAMETERS_SRC_FILES) $(PARAMETERS_PYDEPS) | $(PARAMETERS_GENDIR)
# 	$(call MK_ECHO,Generating parameters.xml/json)
# 	$(PARAM_GEN_CMD)
# 	@touch $@

# $(PARAMETERS_GEN_XML) $(PARAMETERS_GEN_JSON) $(PARAMETERS_GEN_JSON_XZ): $(PARAMETERS_STAMP)
# 	@test -f $@ || { \
# 	    echo "missing $@, regenerating"; \
# 	    $(PARAM_GEN_CMD); \
# 	    touch $(PARAMETERS_STAMP); \
# 	}

$(PARAMETERS_MODULES_FILE) : $(PARAMETERS_MODULE_CFGFILES) $(PARAMETERS_DIR)/module_config/generate_params.py
	$(call MK_ECHO,Generating module_params.c)
	@$(PYTHON_EXECUTABLE) $(PARAMETERS_DIR)/module_config/generate_params.py \
        --params-file $@ \
        --config-files ${PARAMETERS_MODULE_CFGFILES} \
        --board ${PARAMETERS_BOARD}

$(PARAMETERS_GEN_XML): $(PARAMETERS_SRC_FILES) $(PARAMETERS_PYDEPS) | $(PARAMETERS_GENDIR)
	$(call MK_ECHO,Generating parameters.xml/json)
	@$(PYTHON_EXECUTABLE) $(PARAMETERS_DIR)/px_process_params.py \
        --src-path $(PARAMETERS_MODULE_LIST) $(PARAMETERS_GENDIR) \
        --xml $(PARAMETERS_GEN_XML) \
        --json $(PARAMETERS_GEN_JSON) \
        --compress \
        --inject-xml $(PARAMETERS_DIR)/parameters_injected.xml \
        --overrides '$(PARAMETERS_OVERRIDES)' \
        --board $(PARAMETERS_BOARD)

$(PARAMETERS_GEN_JSON) $(PARAMETERS_GEN_JSON_XZ): $(PARAMETERS_GEN_XML)
	@test -f $@ || { \
        echo "missing $@, regenerate"; \
        rm -f $(PARAMETERS_GEN_XML); \
        $(MAKE) $(PARAMETERS_GEN_XML); \
	}

$(PARAMETERS_HPP): $(PARAMETERS_GEN_XML) \
    $(PARAMETERS_DIR)/px_generate_params.py \
    $(PARAMETERS_DIR)/templates/px4_parameters.hpp.jinja
	$(call MK_ECHO,Generating px4_parameters.hpp)
	@$(PYTHON_EXECUTABLE) $(PARAMETERS_DIR)/px_generate_params.py \
        --xml $(PARAMETERS_GEN_XML) \
        --dest $(PARAMETERS_TARGET_DIR)

USER_FILE_PREBUILD += $(PARAMETERS_HPP) $(PARAMETERS_MODULES_FILE)
SRCDEP_FILE        += $(PARAMETERS_HPP) $(PARAMETERS_MODULES_FILE)

ALL_EXT_GROUPS += parameters
parameters_OPT := -Wno-cast-align -Wno-sign-compare -DMODULE_NAME=\"parameters\"
parameters_SRC += ${PX4LIBPATH}/lib/parameters/parameters.cpp
parameters_SRC += ${PX4LIBPATH}/lib/parameters/param_translation.cpp
CPPSOURCES += ${parameters_SRC}

ifeq (${CONFIG_TEST},y)
CPPSOURCES += ${PX4LIBPATH}/lib/parameters/ParameterTest.cpp
endif

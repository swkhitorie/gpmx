
ifeq (${PYTHON_EXECUTABLE},)
PYTHON_EXECUTABLE := python3 
endif

WMM_DIR           := ${SDK_ROOTDIR}/${PX4LIBPATH}/lib/world_magnetic_model
WMM_FETCH_SCRIPT  := $(WMM_DIR)/fetch_noaa_table.py
WMM_GEN_TABLE     := $(WMM_DIR)/geo_magnetic_tables.hpp
WMM_GTEST_SCRIPT  := $(WMM_DIR)/generate_gtest.py
WMM_GEN_GTEST     := $(WMM_DIR)/test_geo_lookup.cpp

# $(WMM_GEN_TABLE): $(WMM_FETCH_SCRIPT) | $(WMM_DIR)
# 	$(call MK_ECHO,Updating world magnetic model from NOAA ($@))
# 	cd $(WMM_DIR) && @$(PYTHON_EXECUTABLE) $(WMM_FETCH_SCRIPT) > $@.tmp && mv $@.tmp $@

# $(WMM_GEN_GTEST): $(WMM_GTEST_SCRIPT) | $(WMM_DIR)
# 	$(call MK_ECHO,Updating world magnetic model unit tests from NOAA ($@))
# 	cd $(WMM_DIR) && @$(PYTHON_EXECUTABLE) $(WMM_GTEST_SCRIPT) > $@.tmp && mv $@.tmp $@

CPPSOURCES += ${PX4LIBPATH}/lib/world_magnetic_model/geo_mag_declination.cpp

ifeq (${CONFIG_TEST},y)
ALL_EXT_GROUPS += test_geo_lookup
test_geo_lookup_OPT := -O0 -Wno-double-promotion
test_geo_lookup_SRC += ${PX4LIBPATH}/lib/world_magnetic_model/test_geo_lookup.cpp
CPPSOURCES += ${test_geo_lookup_SRC}
endif

############################################################################
#
#   Copyright (c) 2017 - 2020 PX4 Development Team. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
# 3. Neither the name PX4 nor the names of its contributors may be
#    used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
# OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
# AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
############################################################################

if (NOT PARAM_DEFAULT_OVERRIDES)
	set(PARAM_DEFAULT_OVERRIDES "{}")
endif()

set(PX_BOARD "default")

set(generated_params_dir ${CMAKE_BINARY_DIR}/generated_params)

list(APPEND module_list /home/miku/__workplace/gpmx/apps/libs/pxlibs/modules/sensors)
list(APPEND module_list /home/miku/__workplace/gpmx/apps/libs/pxlibs/libs/collision_prevention)
list(APPEND module_list /home/miku/__workplace/gpmx/apps/libs/pxlibs/libs/controllib/controllib_test)

set(parameters_xml ${CMAKE_BINARY_DIR}/parameters.xml)
set(parameters_json ${CMAKE_BINARY_DIR}/parameters.json)
file(GLOB_RECURSE param_src_files ${PX4_SOURCE_DIR}/src/*params.c ${PX4_SOURCE_DIR}/src/*parameters.c)
add_custom_command(OUTPUT ${parameters_xml} ${parameters_json} ${parameters_json}.xz
    COMMAND ${CMAKE_COMMAND} -E make_directory ${generated_params_dir}
	COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/px_process_params.py
		--src-path ${module_list} ${generated_params_dir}
		--xml ${parameters_xml}
		--json ${parameters_json}
		--compress
		--inject-xml ${CMAKE_CURRENT_SOURCE_DIR}/parameters_injected.xml
		--overrides ${PARAM_DEFAULT_OVERRIDES}
		--board ${PX4_BOARD}
		#--verbose
	DEPENDS
		${param_src_files}
		parameters_injected.xml
		px4params/srcparser.py
		px4params/srcscanner.py
		px4params/jsonout.py
		px4params/xmlout.py
		px_process_params.py
		parameters_injected.xml
	COMMENT "Generating parameters.xml"
)
add_custom_target(parameters_xml DEPENDS ${parameters_xml})

# generate px4_parameters.hpp
# add_custom_command(OUTPUT px4_parameters.hpp
# 	COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/px_generate_params.py
# 		--xml ${parameters_xml} --dest ${CMAKE_CURRENT_BINARY_DIR}
# 	DEPENDS
# 		${CMAKE_BINARY_DIR}/parameters.xml
# 		px_generate_params.py
# 		templates/px4_parameters.hpp.jinja
#     COMMENT "Generating px4_parameters.hpp"
# )
add_custom_target(parameters_header DEPENDS px4_parameters.hpp)

set(SRCS)

list(APPEND SRCS parameters.cpp)
list(APPEND SRCS param_translation.cpp)

# TODO: find a better way to do this
add_library(parameters STATIC
    ${SRCS}
    px4_parameters.hpp
)

target_link_libraries(parameters PRIVATE tinybson lib_gpm px_platform crc32)
target_include_directories(parameters PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/../)

target_compile_definitions(parameters PRIVATE -DMODULE_NAME="parameters")
target_compile_options(parameters
    PRIVATE
        #-DDEBUG_BUILD
        -Wno-cast-align		# TODO: fix and enable
        -Wno-sign-compare	# TODO: fix and enable
)

if(BUILD_TEST)
    add_unit_test(
        TARGET_NAME px_parameters
        SOURCES ParameterTest.cpp
        LIBRARIES
            parameters
            lib_gpm
            px_platform
            crc32
    )
endif()


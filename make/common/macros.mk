################################################################################
# macros.mk
# This makefiles defines all variables and macros used by build makefiles
################################################################################

################################################################################
# Constants section
################################################################################

ifeq (${MACROS_MAKE},)
#$(warning First call)
MACROS_MAKE:=included

# Build process constants
MK_COMMA:= ,
MK_EMPTY:=
MK_SPACE:= ${MK_EMPTY} ${MK_EMPTY}

################################################################################
# Macros section
################################################################################

# convert path to win path
# 1 - Paths to convert
MK_PATHTOWIN=$(subst /,\,${1})

# convert path to unix path
# 1 - Paths to convert
MK_PATHTOUNX=$(subst \,/,${1})

# Convert spaces to commas
# 1 - string to convert
MK_SPCTOCOM=$(subst ${MK_SPACE},${MK_COMMA},${1})

# Busybox executable
#
ifeq ($(PROCESSOR_ARCHITEW6432),AMD64)
MK_BUSYBOX=$(call MK_PATHTOWIN,${TOOLS_DIR}/busybox64.exe)
else
MK_BUSYBOX=$(call MK_PATHTOWIN,${TOOLS_DIR}/busybox.exe)
endif

# Convert non alphanumeric characters to underlines, remove trailing _ and convert to lower case
# 1 - string to convert
MK_FMTNAME=$(shell @${MK_BUSYBOX} echo ${1} | @${MK_BUSYBOX} sed "s/[^a-zA-Z0-9_]/_/g" | @${MK_BUSYBOX} sed "s/_*$$//g" | @${MK_BUSYBOX} tr "[:upper:]" "[:lower:])

# Convert a windows path to 8.3 format
# 1 - string to convert
MK_SHORTNAME=$(shell ${TOOLS_DIR}/pathtoshort.bat ${1})

# check if file exists
# 1 - File to check
MK_FILEEXIST=$(wildcard $(subst ", ,${1}))

# Echo a string
# 1 - files to type/append
MK_ECHO=$(if ${1},@${MK_BUSYBOX} echo ${1})

# type files/append files to another one
# 1 - files to type/append
# 2 (opt) - destination file
MK_CAT=$(if $(and ${1},${2}),@${MK_BUSYBOX} cat ${1} >> ${2})

# create a folder if it does not exist
# 1 - folder name
MK_MKDIR=$(if ${1},@${MK_BUSYBOX} mkdir -p $(1))

# remove files
# 1 - files to remove
MK_RMFILE=$(if ${1},@${MK_BUSYBOX} rm -f ${1})

# remove files exploding wildcard (check globbing behaviour of Windows port of busybox)
# 1 - files to remove
MK_RMFILE_WC=$(if ${1},@${MK_BUSYBOX} sh -c "list=`ls ${1} 2>/dev/null`; if [ -n \"$$list\" ] ; then rm -f $$list ; fi")

# remove directory
# 1 - dir to remove
MK_RMDIR=$(if ${1},@${MK_BUSYBOX} rm -rf ${1})

# change files to read/write exploding wildcard (check globbing behaviour of Windows port of busybox)
# 1 - files to set read/write
MK_SETFILERW_WC=$(if ${1},@${MK_BUSYBOX} sh -c "list=`ls ${1} 2>/dev/null`; if [ -n \"$$list\" ] ; then chmod -f a+w $$list ; fi")

# copy files
# 1 - source file(s)
# 2 - destination file/folder
MK_CP=$(if $(and ${1},${2}),@${MK_BUSYBOX} cp ${1} ${2})

# append text to a file
# 1 - text to append
# 2 - destination file
MK_APPEND=$(if $(and ${1},${2}),@${MK_BUSYBOX} echo ${1}>>${2})
MK_APPGET=$(if $(and ${1},${2}),@${MK_BUSYBOX} ${1}=$(car ${2}))

# convert text to lowercase
# 1 - text to convert
MK_LOWER=$(if ${1},$(shell @${MK_BUSYBOX} echo ${1} | ${MK_BUSYBOX} tr '[:upper:]' '[:lower:]' ))

# convert text to uppercase
# 1 - text to convert
MK_UPPER=$(if ${1},$(shell @${MK_BUSYBOX} echo ${1} | ${MK_BUSYBOX} tr '[:lower:]' '[:upper:]' ))

# flash a binary image using openocd
# 1 - openocd dir
# 2 - debug config
# 3 - chip config
# 4 - bin
# 5 - load address
MK_FWPROGRAM=${1}\bin\openocd.exe -f ${2} -f ${3} -c init -c halt -c "init; targets; reset halt;\
				wait_halt; poll; flash write_image erase ${4} ${5}; reset run; shutdown" \
				-c reset -c shutdown
else
#$(warning Nested call)
endif


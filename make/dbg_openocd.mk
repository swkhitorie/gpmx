#######################################
# open download (from github)
#######################################

# TC_OPENOCD_PATH := D:\__dev_envir\__env\openocd\xpack-openocd-0.11.0-1

TC_OPENOCD_DEBUG_CFG_PATH := ${TC_OPENOCD_PATH}\scripts\interface

TC_OPENOCD_CHIP_CFG_PATH := ${TC_OPENOCD_PATH}\scripts\target

TC_OPENOCD_DEBUG_CFG := ${TC_OPENOCD_DEBUG_CFG_PATH}\${PROJ_OPENOCD_DEBUG}.cfg

TC_OPENOCD_CHIP_CFG := ${TC_OPENOCD_CHIP_CFG_PATH}\${PROJ_OPENOCD_CHIP}.cfg


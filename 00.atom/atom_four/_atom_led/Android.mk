
#LOCAL_PATH:=$(call my-dir) 

#include $(CLEAR_VARS) 


LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_ARM_MODE:=arm 
LOCAL_MODULE_TAGS := optional
# LOCAL_SHARED_LIBRARIES:= libc libcutils libnvram libcustom_nvram libfile_op 

LOCAL_SRC_FILES := $(call all-subdir-c-files)
# LOCAL_SRC_FILES:= cust_daemon.c 

# LOCAL_C_INCLUDES += mediatek/custom/$(TARGET_PRODUCT)/cgen/cfgdefault
# LOCAL_C_INCLUDES += mediatek/custom/$(TARGET_PRODUCT)/cgen/cfgfileinc
# LOCAL_C_INCLUDES += mediatek/custom/$(TARGET_PRODUCT)/cgen/inc

# LOCAL_C_INCLUDES:= \
		$(MTK_PATH_SOURCE)/external/nvram/libnvram \
		$(MTK_PATH_SOURCE)/external/nvram/libfile_op \
		$(MTK_PATH_CUSTOM)/cgen/inc \
		$(MTK_PATH_CUSTOM)/cgen/cfgfileinc \
		$(MTK_PATH_CUSTOM)/cgen/cfgdefault \
		$(MTK_PATH_SOURCE)/external/mtd_util \
		$(MTK_PATH_SOURCE)/kernel/include/linux
# 
# LOCAL_STATIC_LIBRARIES := libmtd
LOCAL_MODULE:= _atom_led 

include $(BUILD_EXECUTABLE) 



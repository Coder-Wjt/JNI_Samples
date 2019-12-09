LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := astra_android_bridge
LOCAL_SRC_FILES := astra_android_bridge.cpp

LOCAL_C_INCLUDES	:= $(LOCAL_PATH)/include
LOCAL_CPPFLAGS 	:= -frtti -fexceptions
LOCAL_LDLIBS := -llog
include $(BUILD_SHARED_LIBRARY)

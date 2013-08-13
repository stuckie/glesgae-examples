LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := native-activity

FILE_LIST := $(wildcard $(LOCAL_PATH)/../*.cpp)
LOCAL_SRC_FILES := $(FILE_LIST:$(LOCAL_PATH)/%=%)

LOCAL_CFLAGS	:= -DGLES1 -DANDROID
LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv1_CM
LOCAL_SHARED_LIBRARIES := android_native_app_glue
LOCAL_SHARED_LIBRARIES += glesgae

include $(BUILD_SHARED_LIBRARY)

$(call import-module,glesgae)
$(call import-module,android/native_app_glue)

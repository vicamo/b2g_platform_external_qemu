LOCAL_PATH := $(call my-dir)

host_OS_SRCS :=
host_common_LDLIBS :=

ifeq ($(BUILD_TARGET_OS),linux)
    host_OS_SRCS = NativeSubWindow_x11.cpp
    host_common_LDLIBS += -lX11 -lrt
endif

ifeq ($(BUILD_TARGET_OS),darwin)
    host_OS_SRCS = NativeSubWindow_cocoa.m
    host_common_LDLIBS += -Wl,-framework,AppKit
endif

ifeq ($(BUILD_TARGET_OS),windows)
    host_OS_SRCS = NativeSubWindow_win32.cpp
    host_common_LDLIBS += -lgdi32
endif

host_common_SRC_FILES := \
    $(host_OS_SRCS) \
    ChannelStream.cpp \
    ColorBuffer.cpp \
    FbConfig.cpp \
    FenceSyncInfo.cpp \
    FrameBuffer.cpp \
    ReadBuffer.cpp \
    RenderChannelImpl.cpp \
    RenderContext.cpp \
    RenderControl.cpp \
    RendererImpl.cpp \
    RenderLibImpl.cpp \
    RenderThread.cpp \
    RenderThreadInfo.cpp \
    render_api.cpp \
    RenderWindow.cpp \
    SyncThread.cpp \
    TextureDraw.cpp \
    TextureResize.cpp \
    TimeUtils.cpp \
    WindowSurface.cpp \

### host libOpenglRender #################################################
$(call emugl-begin-shared-library,lib$(BUILD_TARGET_SUFFIX)OpenglRender)

$(call emugl-import,libGLESv1_dec libGLESv2_dec lib_renderControl_dec libOpenglCodecCommon)

LOCAL_LDLIBS += $(host_common_LDLIBS)

LOCAL_SRC_FILES := $(host_common_SRC_FILES)
$(call emugl-export,C_INCLUDES,$(EMUGL_PATH)/host/include)
$(call emugl-export,C_INCLUDES,$(LOCAL_PATH))

# use Translator's egl/gles headers
LOCAL_C_INCLUDES += $(EMUGL_PATH)/host/libs/Translator/include
LOCAL_C_INCLUDES += $(EMUGL_PATH)/host/libs/libOpenGLESDispatch

LOCAL_STATIC_LIBRARIES += libemugl_common
LOCAL_STATIC_LIBRARIES += libOpenGLESDispatch

LOCAL_SYMBOL_FILE := render_api.entries

$(call emugl-export,CFLAGS,$(EMUGL_USER_CFLAGS))

$(call emugl-end-module)

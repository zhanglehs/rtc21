LOCAL_PATH := $(call my-dir)



include $(CLEAR_VARS)
LOCAL_MODULE    := libavengine_dll
LOCAL_SRC_FILES := prebuilt/libavengine_dll.so
include $(PREBUILT_SHARED_LIBRARY)



include $(CLEAR_VARS)
LOCAL_MODULE    := engine_sdk_jni
LOCAL_SHARED_LIBRARIES += libavengine_dll

ENGINE_SDK_DIR = ../../../..
#LIB_EVENT_DIR = $(LOCAL_PATH)/../libevent-2.1.8
LIB_EVENT_DIR = $(ENGINE_SDK_DIR)/third_party/libevent/android

LOCAL_CPPFLAGS += -fvisibility=hidden -fexceptions -std=c++11 -DANDROID -pthread -frtti -DHAVE_PTHREAD=1
LOCAL_C_INCLUDES := -I $(ENGINE_SDK_DIR)/src -I $(ENGINE_SDK_DIR)/src/util/jsoncpp/include -I $(LIB_EVENT_DIR)/include

LOCAL_LDLIBS += $(LIB_EVENT_DIR)/lib/libevent.a
LOCAL_LDLIBS += $(LIB_EVENT_DIR)/lib/libevent_core.a
LOCAL_LDLIBS += $(LIB_EVENT_DIR)/lib/libevent_extra.a
LOCAL_LDLIBS += $(LIB_EVENT_DIR)/lib/libevent_pthreads.a
LOCAL_LDLIBS += -lz -llog

ENGINE_SDK_SRC =$(ENGINE_SDK_DIR)/src
LOCAL_SRC_FILES = $(ENGINE_SDK_SRC)/avformat/rtcp_dec.cpp
LOCAL_SRC_FILES += $(ENGINE_SDK_SRC)/avformat/rtcp_enc.cpp
LOCAL_SRC_FILES += $(ENGINE_SDK_SRC)/avformat/rtcp_helper.cpp
LOCAL_SRC_FILES += $(ENGINE_SDK_SRC)/cmd_protocol/proto_common.cpp
LOCAL_SRC_FILES += $(ENGINE_SDK_SRC)/cmd_protocol/proto_rtp_rtcp.cpp
LOCAL_SRC_FILES += $(ENGINE_SDK_SRC)/download/rtp_download.cpp
LOCAL_SRC_FILES += $(ENGINE_SDK_SRC)/download/rtp_download_internal.cpp
LOCAL_SRC_FILES += $(ENGINE_SDK_SRC)/log/log.cpp
LOCAL_SRC_FILES += $(ENGINE_SDK_SRC)/media_manager/rtp_block.cpp
LOCAL_SRC_FILES += $(ENGINE_SDK_SRC)/media_manager/rtp_block_cache.cpp
LOCAL_SRC_FILES += $(ENGINE_SDK_SRC)/media_manager/rtp_media_manager.cpp
LOCAL_SRC_FILES += $(ENGINE_SDK_SRC)/network/CHttpFetch.cpp
LOCAL_SRC_FILES += $(ENGINE_SDK_SRC)/network/network_channel.cpp
LOCAL_SRC_FILES += $(ENGINE_SDK_SRC)/network/tcp.cpp
LOCAL_SRC_FILES += $(ENGINE_SDK_SRC)/network/udp.cpp
LOCAL_SRC_FILES += $(ENGINE_SDK_SRC)/rtp_trans/rtp_receiver_session.cpp
LOCAL_SRC_FILES += $(ENGINE_SDK_SRC)/rtp_trans/rtp_receiver_trans.cpp
LOCAL_SRC_FILES += $(ENGINE_SDK_SRC)/rtp_trans/rtp_sender_session.cpp
LOCAL_SRC_FILES += $(ENGINE_SDK_SRC)/rtp_trans/rtp_sender_trans.cpp
LOCAL_SRC_FILES += $(ENGINE_SDK_SRC)/rtp_trans/rtp_session.cpp
LOCAL_SRC_FILES += $(ENGINE_SDK_SRC)/rtp_trans/rtp_trans.cpp
LOCAL_SRC_FILES += $(ENGINE_SDK_SRC)/rtp_trans/send_side_bandwidth_estimation.cc
LOCAL_SRC_FILES += $(ENGINE_SDK_SRC)/sdk/rtp_format.cpp
LOCAL_SRC_FILES += $(ENGINE_SDK_SRC)/sdp/aac.cpp
LOCAL_SRC_FILES += $(ENGINE_SDK_SRC)/sdp/bits.cpp
LOCAL_SRC_FILES += $(ENGINE_SDK_SRC)/sdp/sdp.cpp
LOCAL_SRC_FILES += $(ENGINE_SDK_SRC)/superlogic/api.cc
LOCAL_SRC_FILES += $(ENGINE_SDK_SRC)/superlogic/RtcCapture.cc
LOCAL_SRC_FILES += $(ENGINE_SDK_SRC)/superlogic/RtcCaptureInternal.cc
LOCAL_SRC_FILES += $(ENGINE_SDK_SRC)/superlogic/RtcPlayer.cc
LOCAL_SRC_FILES += $(ENGINE_SDK_SRC)/superlogic/RtcPlayerInternal.cc
LOCAL_SRC_FILES += $(ENGINE_SDK_SRC)/superlogic/webrtc_base.cc
LOCAL_SRC_FILES += $(ENGINE_SDK_SRC)/upload/rtp_package.cc
LOCAL_SRC_FILES += $(ENGINE_SDK_SRC)/upload/rtp_upload.cpp
LOCAL_SRC_FILES += $(ENGINE_SDK_SRC)/upload/rtp_upload_internal.cpp
LOCAL_SRC_FILES += $(ENGINE_SDK_SRC)/util/base_exception.cpp
LOCAL_SRC_FILES += $(ENGINE_SDK_SRC)/util/data_buffer.cpp
LOCAL_SRC_FILES += $(ENGINE_SDK_SRC)/util/util_common.cpp
LOCAL_SRC_FILES += $(ENGINE_SDK_SRC)/util/jsoncpp/src/lib_json/json_reader.cpp
LOCAL_SRC_FILES += $(ENGINE_SDK_SRC)/util/jsoncpp/src/lib_json/json_value.cpp
LOCAL_SRC_FILES += $(ENGINE_SDK_SRC)/util/jsoncpp/src/lib_json/json_writer.cpp
LOCAL_SRC_FILES += src/LFListener.cpp
LOCAL_SRC_FILES += src/com_laifeng_rtp_upload.cpp
LOCAL_SRC_FILES += src/com_laifeng_rtp_player.cpp

include $(BUILD_SHARED_LIBRARY)

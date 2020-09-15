#ifndef MSC_TEST_MEDIA_STREAM_TRACK_FACTORY_HPP
#define MSC_TEST_MEDIA_STREAM_TRACK_FACTORY_HPP

#include "api/media_stream_interface.h"

rtc::scoped_refptr<webrtc::AudioTrackInterface> createAudioTrack();

rtc::scoped_refptr<webrtc::VideoTrackInterface> createVideoTrack();

#endif

#define MSC_CLASS "MediaStreamTrackFactory"

#include <iostream>

#include "MediaStreamTrackFactory.hpp"
#include "MediaSoupClientErrors.hpp"

using namespace mediasoupclient;

#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
#include "api/create_peerconnection_factory.h"
#include "api/video_codecs/builtin_video_decoder_factory.h"
#include "api/video_codecs/builtin_video_encoder_factory.h"
#include "api/video/video_frame.h"
#include "api/video/video_source_interface.h"
#include "api/video/i420_buffer.h"
#include "api/video/video_frame_buffer.h"
#include "api/video/video_rotation.h"
#include "api/scoped_refptr.h"
#include "media/base/video_adapter.h"
#include "media/base/video_broadcaster.h"
#include "modules/video_capture/video_capture.h"
#include "modules\video_capture\video_capture_factory.h"
#include "pc/video_track_source.h"

static rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory;

/* MediaStreamTrack holds reference to the threads of the PeerConnectionFactory.
 * Use plain pointers in order to avoid threads being destructed before tracks.
 */
static rtc::Thread* networkThread;
static rtc::Thread* signalingThread;
static rtc::Thread* workerThread;

static void createFactory()
{
	networkThread   = rtc::Thread::Create().release();
	signalingThread = rtc::Thread::Create().release();
	workerThread    = rtc::Thread::Create().release();

	networkThread->SetName("network_thread", nullptr);
	signalingThread->SetName("signaling_thread", nullptr);
	workerThread->SetName("worker_thread", nullptr);

	if (!networkThread->Start() || !signalingThread->Start() || !workerThread->Start())
	{
		MSC_THROW_INVALID_STATE_ERROR("thread start errored");
	}

	factory = webrtc::CreatePeerConnectionFactory(
	  networkThread,
	  workerThread,
	  signalingThread,
	  nullptr,
	  webrtc::CreateBuiltinAudioEncoderFactory(),
	  webrtc::CreateBuiltinAudioDecoderFactory(),
	  webrtc::CreateBuiltinVideoEncoderFactory(),
	  webrtc::CreateBuiltinVideoDecoderFactory(),
	  nullptr /*audio_mixer*/,
	  nullptr /*audio_processing*/);

	if (!factory)
	{
		MSC_THROW_ERROR("error ocurred creating peerconnection factory");
	}
}

// Audio track creation.
rtc::scoped_refptr<webrtc::AudioTrackInterface> createAudioTrack()
{
	if (!factory)
		createFactory();

	cricket::AudioOptions options;
	options.highpass_filter = false;

	rtc::scoped_refptr<webrtc::AudioSourceInterface> source = factory->CreateAudioSource(options);
	return factory->CreateAudioTrack(rtc::CreateRandomUuid(), source);
}

class TestVideoCapturer : public rtc::VideoSourceInterface<webrtc::VideoFrame> {
public:
	TestVideoCapturer() = default;
	virtual ~TestVideoCapturer() = default;

	void AddOrUpdateSink(rtc::VideoSinkInterface<webrtc::VideoFrame>* sink, const rtc::VideoSinkWants& wants) override {
		broadcaster_.AddOrUpdateSink(sink, wants);
		UpdateVideoAdapter();
	}

	void RemoveSink(rtc::VideoSinkInterface<webrtc::VideoFrame>* sink) override {
		broadcaster_.RemoveSink(sink);
		UpdateVideoAdapter();
	}

protected:
	void OnFrame(const webrtc::VideoFrame& frame) {
		int cropped_width = 0;
		int cropped_height = 0;
		int out_width = 0;
		int out_height = 0;

		if (!video_adapter_.AdaptFrameResolution(frame.width(), frame.height(), frame.timestamp_us() * 1000,&cropped_width, &cropped_height, &out_width, &out_height)) {
			// Drop frame in order to respect frame rate constraint.
			return;
		}

		if (out_height != frame.height() || out_width != frame.width()) {
			// Video adapter has requested a down-scale. Allocate a new buffer and
			// return scaled version.
			rtc::scoped_refptr<webrtc::I420Buffer> scaled_buffer = webrtc::I420Buffer::Create(out_width, out_height);
			scaled_buffer->ScaleFrom(*frame.video_frame_buffer()->ToI420());
			broadcaster_.OnFrame(
				webrtc::VideoFrame::Builder()
				.set_video_frame_buffer(scaled_buffer)
				.set_rotation(webrtc::kVideoRotation_0)
				.set_timestamp_us(frame.timestamp_us())
				.set_id(frame.id())
				.build());
		}
		else {
			// No adaptations needed, just return the frame as is.
			broadcaster_.OnFrame(frame);
		}
	}

	rtc::VideoSinkWants GetSinkWants() {
		return broadcaster_.wants();
	}

private:
	void UpdateVideoAdapter() {
		rtc::VideoSinkWants wants = broadcaster_.wants();
		video_adapter_.OnResolutionFramerateRequest(wants.target_pixel_count, wants.max_pixel_count, wants.max_framerate_fps);
	}

	rtc::VideoBroadcaster broadcaster_;
	cricket::VideoAdapter video_adapter_;
};

class VcmCapturer : public TestVideoCapturer,
	public rtc::VideoSinkInterface<webrtc::VideoFrame> {
public:
	static VcmCapturer* Create(size_t width, size_t height, size_t target_fps, size_t capture_device_index) {
		
		std::unique_ptr<VcmCapturer> vcm_capturer(new VcmCapturer());
		if (!vcm_capturer->Init(width, height, target_fps, capture_device_index)) {
			RTC_LOG(LS_WARNING) << "Failed to create VcmCapturer(w = " << width << ", h = " << height << ", fps = " << target_fps << ")";
			return nullptr;
		}

		return vcm_capturer.release();
	}

	virtual ~VcmCapturer() {
		Destroy();
	}

	void OnFrame(const webrtc::VideoFrame& frame) override {
		TestVideoCapturer::OnFrame(frame);
	}

private:
	VcmCapturer() : vcm_(nullptr) {}

	bool Init(size_t width, size_t height, size_t target_fps, size_t capture_device_index) {
		std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> device_info(webrtc::VideoCaptureFactory::CreateDeviceInfo());

		char device_name[256];
		char unique_name[256];
		if (device_info->GetDeviceName(static_cast<uint32_t>(capture_device_index),
			device_name, sizeof(device_name), unique_name,
			sizeof(unique_name)) != 0) {
			Destroy();
			return false;
		}

		vcm_ = webrtc::VideoCaptureFactory::Create(unique_name);
		if (!vcm_) {
			return false;
		}
		vcm_->RegisterCaptureDataCallback(this);

		device_info->GetCapability(vcm_->CurrentDeviceName(), 0, capability_);

		capability_.width = static_cast<int32_t>(width);
		capability_.height = static_cast<int32_t>(height);
		capability_.maxFPS = static_cast<int32_t>(target_fps);
		capability_.videoType = webrtc::VideoType::kI420;

		if (vcm_->StartCapture(capability_) != 0) {
			Destroy();
			return false;
		}

		RTC_CHECK(vcm_->CaptureStarted());
		return true;
	}
	
	void Destroy() {
		if (!vcm_)
			return;

		vcm_->StopCapture();
		vcm_->DeRegisterCaptureDataCallback();
		// Release reference to VCM.
		vcm_ = nullptr;
	}

	rtc::scoped_refptr<webrtc::VideoCaptureModule> vcm_;
	webrtc::VideoCaptureCapability capability_;
};


class CapturerTrackSource : public webrtc::VideoTrackSource {
public:
	static rtc::scoped_refptr<CapturerTrackSource> Create() {
		const size_t kWidth = 640;
		const size_t kHeight = 480;
		const size_t kFps = 30;

		std::unique_ptr<VcmCapturer> capturer;
		std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> info(webrtc::VideoCaptureFactory::CreateDeviceInfo());

		if (!info) {
			return nullptr;
		}

		int num_devices = info->NumberOfDevices();

		for (int i = 0; i < num_devices; ++i) {
			capturer = absl::WrapUnique(VcmCapturer::Create(kWidth, kHeight, kFps, i));
			if (capturer) {
				return new
					rtc::RefCountedObject<CapturerTrackSource>(std::move(capturer));
			}
		}

		return nullptr;
	}

protected:
	explicit CapturerTrackSource(std::unique_ptr<VcmCapturer> capturer)
		: VideoTrackSource(/*remote=*/false), capturer_(std::move(capturer)) {}

private:
	rtc::VideoSourceInterface<webrtc::VideoFrame>* source() override {
		return capturer_.get();
	}

	std::unique_ptr<VcmCapturer> capturer_;
};


// Video track creation.
rtc::scoped_refptr<webrtc::VideoTrackInterface> createVideoTrack()
{
	if (!factory)
		createFactory();

	return factory->CreateVideoTrack(rtc::CreateRandomUuid(), CapturerTrackSource::Create());
}

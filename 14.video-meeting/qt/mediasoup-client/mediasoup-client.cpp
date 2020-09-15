// mediasoup-client.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include "mediasoupclient.hpp"
using namespace mediasoupclient;
using namespace nlohmann;

#include "MediaStreamTrackFactory.hpp"
#include "api/video/i420_buffer.h"
#include "third_party/libyuv/include/libyuv/convert_argb.h"
#include "rtc_base/arraysize.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"

#include <iostream>
#include <iomanip>
#include <string>
using namespace std;


#include <boost/process.hpp>
namespace bp = boost::process;

#include <boost/format.hpp>
using boost::format;

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
namespace bi = boost::interprocess;

int main()
{
	bp::ipstream in_signaling_client;
	bp::opstream out_signaling_client;
	
	bp::child signaling_client(
		bp::search_path(R"(signaling-client)"),
		bp::std_out > in_signaling_client,
		bp::std_in < out_signaling_client
	);

    Initialize();
	auto device = new Device();

	struct SendTransportListener : public SendTransport::Listener, public Producer::Listener {
		pair<bp::opstream&, bp::ipstream&> io;

		SendTransportListener(decltype(io) io) :io(io) {}

		future<void> OnConnect(Transport* transport, const json& dtlsParameters) {
			promise<void> res;
			
			string connected;
			auto args = format(R"({ "dtlsParameters":%1% })") % dtlsParameters.dump();
			io.first << "connectProducerTransport" << endl << args.str() << endl;
			getline(io.second,connected);
			res.set_value();

			return res.get_future();
		}

		void OnConnectionStateChange(Transport* transport, const string& connectionState) {
			if (connectionState == "connecting")
				cout << "connecting..." << endl;
			else if ("connected") 
				cout << "connected" << endl;
			else if ("failed")
				transport->Close(), cout << "failed";
		}

		future<string> OnProduce(SendTransport* transport, const string& kind, json rtpParameters, const json& appData) {
			promise<string> res;
			
			auto args = format(R"({ "transportId": "%1%","kind":"%2%", "rtpParameters": %3% })") % transport->GetId() % kind % rtpParameters.dump();
			io.first << "produce" << endl << args.str() << endl;

			string produce_id;
			getline(io.second, produce_id);
			res.set_value(produce_id);

			return res.get_future();
		}

		void OnTransportClose(Producer* producer) {
			cout << "transport close." << endl;
		}
	};

	SendTransportListener send_listener({ out_signaling_client,in_signaling_client });

	struct RecvTransportListener : public RecvTransport::Listener, public Consumer::Listener {
		pair<bp::opstream&, bp::ipstream&> io;

		RecvTransportListener(decltype(io) io) :io(io) {}

		future<void> OnConnect(Transport* transport, const json& dtlsParameters) {
			promise<void> res;

			string connected;
			auto args = format(R"({ "transportId":"%1%", "dtlsParameters":%2% })") % transport->GetId() % dtlsParameters.dump();
			io.first << "connectConsumerTransport" << endl << args.str() << endl;
			getline(io.second, connected);
			cout << "onConnect:" << connected << endl;

			res.set_value();

			return res.get_future();
		}

		void OnConnectionStateChange(Transport* transport, const string& connectionState) {
			if (connectionState == "connecting")
				cout << "connecting..." << endl;
			else if ("connected") {
				io.first << "resume" << endl;
				string played;
				getline(io.second, played);
				cout << "connected:resume" << endl;
			}
			else if ("failed")
				transport->Close(), cout << "failed";
		}

		void OnTransportClose(Consumer* producer) {
			cout << "transport close." << endl;
		}
	};

	RecvTransportListener recv_listener({ out_signaling_client,in_signaling_client });

	class VideoRenderer : public rtc::VideoSinkInterface<webrtc::VideoFrame> {
		bi::shared_memory_object shm_obj;

	public:
		VideoRenderer() : shm_obj(bi::open_or_create,"shared_memory",bi::read_write) {
			
		}

		virtual ~VideoRenderer() {
			shm_obj.remove("shared_memory");
		}

		virtual void OnFrame(const webrtc::VideoFrame& video_frame) override {
			cout << '1';
			auto buffer = video_frame.video_frame_buffer()->ToI420();
			
			auto size_header = 2 * sizeof(int);
			auto size = buffer->width() * buffer->height() * 4;

			bi::offset_t s;
			shm_obj.get_size(s);
			if(s < size) shm_obj.truncate(size_header + size);

			bi::mapped_region region_header(shm_obj, bi::read_write,0,size_header);
			auto header = (unsigned int*)region_header.get_address();
			header[0] = buffer->width();
			header[1] = buffer->height();

			bi::mapped_region region(shm_obj,bi::read_write);
			auto image = reinterpret_cast<unsigned char*>(region.get_address()) + size_header;

			cout << "rgb:" << libyuv::I420ToARGB(buffer->DataY(), buffer->StrideY(),
				buffer->DataU(), buffer->StrideU(),
				buffer->DataV(), buffer->StrideV(),
				image, buffer->width() * 4,
				buffer->width(), buffer->height()) << endl;
		}
	}vr;


	string command;
	out_signaling_client << "getRouterRtpCapabilities" << endl;
	
	string routerCapabilities;
	getline(in_signaling_client, routerCapabilities);
	
	cout << "load " << routerCapabilities << endl;
	device->Load(nlohmann::json::parse(routerCapabilities));
	
	do {
		try {
			cout << "in:" << endl;
			getline(cin, command);
			if (command == "publishing") {
				if (device->CanProduce("video")) {
					cout << "can produce video: true" << endl;
				}

				auto track = createVideoTrack();

				auto  device_rtp_capabilities = device->GetRtpCapabilities().dump();
				auto args = format(R"({ "forceTcp": false,"rtpCapabilities":%1% })") % device_rtp_capabilities;
				out_signaling_client << "createProducerTransport" << endl << args.str() << endl;

				string data;
				getline(in_signaling_client, data);
				cout << data << endl;
				auto tp_data = json::parse(data);
				auto send_transport = device->CreateSendTransport(&send_listener, tp_data["id"], tp_data["iceParameters"], tp_data["iceCandidates"], tp_data["dtlsParameters"]);
				auto producer = send_transport->Produce(&send_listener, track, nullptr, nullptr);
			}
			else if (command == "subscription") {
				auto args = format(R"({ "forceTcp": false })");
				out_signaling_client << "createConsumerTransport" << endl << args.str() << endl;
				string data;
				getline(in_signaling_client, data);
				cout << data << endl;
				auto tp_data = json::parse(data);
				auto recv_transport = device->CreateRecvTransport(&recv_listener, tp_data["id"], tp_data["iceParameters"], tp_data["iceCandidates"], tp_data["dtlsParameters"]);
;
				auto  device_rtp_capabilities = device->GetRtpCapabilities().dump();
				args = format(R"({ "rtpCapabilities":%1% })") % device_rtp_capabilities;
				out_signaling_client << "consume" << endl << args.str() << endl;
				data.clear();
				getline(in_signaling_client, data);
				cout << data << endl;
				tp_data = json::parse(data);

				auto consume = recv_transport->Consume(&recv_listener, tp_data["id"], tp_data["producerId"], tp_data["kind"], &tp_data["rtpParameters"]);
				auto track = consume->GetTrack();
				
				cout << track->kind() << endl;
				auto x = static_cast<webrtc::VideoTrackInterface*>(track);
				x->AddOrUpdateSink(&vr, rtc::VideoSinkWants());
				x->Release();
			}
		}
		catch (exception ex) {
			cout << ex.what() << endl;
		}

	} while (command != "exit");
	

	Cleanup();
	return 0;
}
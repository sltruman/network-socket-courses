#pragma once

#include "sio_client.h"
#include "internal/sio_packet.h"

#include <mutex>
#include <sstream>
#include <future>

sio::packet_manager manager;
std::mutex packetLock;

std::string getJson(sio::message::ptr msg)
{
	std::lock_guard< std::mutex > guard(packetLock);
	std::stringstream ss;
	sio::packet packet("/", msg);
	manager.encode(packet, [&](bool isBinary, std::shared_ptr<const std::string> const& json)
	{
		ss << *json;
		assert(!isBinary);
	});
	manager.reset();

	// Need to strip off the message type flags (typically '42',
	// but there are other possible combinations).
	std::string result = ss.str();
	std::size_t indexList = result.find('[');
	std::size_t indexObject = result.find('{');
	std::size_t indexString = result.find('"');
	std::size_t index = indexList;
	if (indexObject != std::string::npos && indexObject < index)
		index = indexObject;
	if (indexString != std::string::npos && indexString < index)
		index = indexString;

	if (index == std::string::npos) {
		std::stringstream err; err << "Error decoding json object " << std::endl << " Body: " << result << std::endl;
		throw std::exception(err.str().c_str());
	}
	return result.substr(index);
}

sio::message::ptr getMessage(const std::string& json)
{
	std::lock_guard< std::mutex > guard(packetLock);
	sio::message::ptr message;
	manager.set_decode_callback([&](sio::packet const& p)
	{
		message = p.get_message();
	});

	// Magic message type / ID
	std::string payload = std::string("42") + json;
	manager.put_payload(payload);

	manager.reset();
	return message;
}


#include <iostream>
using namespace std;

class signaling_client
{
public:
	signaling_client() {
		promise<void> res;

		io.set_open_listener([&]() {
			res.set_value();
		});

		io.set_close_listener([&](sio::client::close_reason const& reason) {
		});

		io.set_fail_listener([&]() {
			exit(0);
		});

		io.connect("https://dungbeetles.xyz");
		res.get_future().wait();
	}

	~signaling_client() {
		io.socket()->close();
		io.sync_close();
	}

	string get_router_rtp_capabilities() {
		promise<string> res;

		io.socket()->emit("getRouterRtpCapabilities", { "" }, [&](auto& data) {
			res.set_value(getJson(data[0]));
		});

		return res.get_future().get();
	}

	string create_producer_transport(string args) {
		promise<string> res;
				
		io.socket()->emit("createProducerTransport", { args }, [&](const sio::message::list & data) {
			res.set_value(getJson(data[0]));
		});

		return res.get_future().get();
	}

	bool connect_producer_transport(string dtls_parameters) {
		promise<bool> res;

		io.socket()->emit("connectProducerTransport", { dtls_parameters } , [&](const sio::message::list & data) {
			res.set_value(true);
		});

		return res.get_future().get();
	}

	string produce(string args) {
		promise<string> res;

		io.socket()->emit("produce", { args }, [&](const sio::message::list & data) {
			res.set_value(getJson(data[0]));
		});

		return res.get_future().get();
	}

	string create_consumer_transport(string args) {
		promise<string> res;

		io.socket()->emit("createConsumerTransport", { args }, [&](const sio::message::list & data) {
			res.set_value(getJson(data[0]));
		});

		return res.get_future().get();
	}

	bool connect_consumer_transport(string dtls_parameters) {
		promise<bool> res;

		io.socket()->emit("connectConsumerTransport", { dtls_parameters }, [&](const sio::message::list & data) {
			res.set_value(true);
		});

		return res.get_future().get();
	}

	string consume(string args) {
		promise<string> res;

		io.socket()->emit("consume", { args }, [&](const sio::message::list & data) {
			res.set_value(getJson(data[0]));
		});

		return res.get_future().get();
	}

	bool resume() {
		promise<bool> res;

		io.socket()->emit("resume", { "" }, [&](const sio::message::list & data) {
			res.set_value(true);
		});

		return res.get_future().get();
	}
private:
	sio::client io;

};


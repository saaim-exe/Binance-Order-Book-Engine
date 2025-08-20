#pragma once
#include "book.h"
#include "fifo.h"
#include "ticks.hpp"
#include <binapi/api.hpp>
#include <binapi/enums.hpp>
#include <binapi/websocket.hpp>
#include <boost/asio/io_context.hpp>
#include <iostream>

namespace Client {

	

	// extern binapi::ws::part_depths_t o;

	extern boost::asio::io_context ioctx;

	extern binapi::ws::websockets ws;



	void getPartialDepth(std::map<double, double>& bids, std::map<double, double>& asks, std::unique_ptr<OrderBook>& book, std::unique_ptr<FifoMatch>& match);
	std::unique_ptr<Order> deserialize(OrderBook& book, double price, double quantity, Side side);


}



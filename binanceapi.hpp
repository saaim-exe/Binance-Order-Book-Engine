#pragma once


#include "book.h"
#include "fifo.h"
#include <binapi/api.hpp>
#include <binapi/enums.hpp>
#include <binapi/websocket.hpp>
#include <boost/asio/io_context.hpp>
#include <iostream>


namespace Client {

	// extern binapi::ws::part_depths_t o;

	extern boost::asio::io_context ioctx;

	extern binapi::ws::websockets ws;
	


	void getPartialDepth(std::map<double, double>& bids, std::map<double, double>& asks, std::unique_ptr<OrderBook> &book, std::unique_ptr<FifoMatch>& match);
	std::unique_ptr<Order> deserialize(OrderBook& book, double price, double quantity, Side side);
	
	 
}



/*#define ASIO_STANDALONE
#define _WEBSOCKETPP_CPP11_STL_
#define _WEBSOCKETPP_CPP11_MEMORY_
#define _WEBSOCKETPP_CPP11_RANDOM_DEVICE_

#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <iostream>

typedef websocketpp::client<websocketpp::config::asio_tls_client> client; 
typedef websocketpp::lib::shared_ptr <websocketpp::lib::asio::ssl::context> context_ptr; 

using websocketpp::lib::placeholders::_1; 
using websocketpp::lib::placeholders::_2; 
using websocketpp::lib::bind; 



class BinanceAPI {

public:

	void on_open(websocketpp::connection_hdl hdl, client* c);
	
	void on_message(websocketpp::connection_hdl, client::message_ptr msg);

	void on_fail(websocketpp::connection_hdl hdl);

	void on_close(websocketpp::connection_hdl hdl); 


private: 


};
*/



	


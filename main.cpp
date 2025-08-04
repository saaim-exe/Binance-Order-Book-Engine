#include <iostream>
#include <vector>
#include <algorithm>
#include <cstddef>
#include <map>
#include <unordered_map>
#include <thread>
#include <string>
#include "order.h"
#include "book.h"
#include "fifo.h"
#include "binanceapi.hpp"
#include "parsedata.hpp"

using namespace Client; 

int main() {

	std::map<double, double> bids;
	std::map<double, double> asks; 

	auto book = std::make_unique<OrderBook>(); 
	auto match = std::make_unique<FifoMatch>();
	
	try {
		Client::getPartialDepth(bids, asks, book, match); 

		std::thread displayThread([&book]() {


			while (true)
			{
				std::this_thread::sleep_for(std::chrono::seconds(1));
				system("cls");
				// book->displayOrder();
				// OrderBooktoFile(book);
			}

			}); 

		Client::ioctx.run(); 
		displayThread.join(); 
	}
	catch (const std::exception& ex)
	{

		std::cerr << "std::exception: " << ex.what() << std::endl; 
	}
	



	



	//std::unique_ptr<Order> orderObjBuy = makeOrder(1, 10.00, 100, Side::BUY, "10:00:00", OrderType::LIMIT);

	
	//std::string serialize = orderObjBuy->serializeOrder(); 

	// std::cout << serialize << std::endl; 


	/*
	OrderID orderId = 111; 
	Price price = 11.24; 

	// int32convertertoDouble(price); 

	Quantity quantity = 30; 
	Timestamp timestamp = "5:35:05"; 



	// Order order(orderId, price, quantity, Side::BUY, timestamp);

	// std::unique_ptr<Order> orderObjBuy = std::make_unique<Order>(orderId, price, quantity, Side::BUY, timestamp);

	



	std::unique_ptr<OrderBook> book = std::make_unique<OrderBook>(); 


	// BUY ORDERS




	book->addOrder(std::move(makeOrder(1, 10.00, 100, Side::BUY, "10:00:00", OrderType::LIMIT))); 
	book->addOrder(std::move(makeOrder(2, 10.50, 50, Side::BUY, "10:01:00", OrderType::LIMIT))); 
	book->addOrder(std::move(makeOrder(3, 9.75, 75, Side::BUY, "10:02:00", OrderType::MARKET))); 


	// SELL ORDERS 

	book->addOrder(std::move(makeOrder(4, 9.80, 80, Side::SELL, "10:03:00", OrderType::LIMIT))); 
	book->addOrder(std::move(makeOrder(5, 10.25, 60, Side::SELL, "10:04:00", OrderType::MARKET))); 
	book->addOrder(std::move(makeOrder(6, 10.50, 40, Side::SELL, "10:05:00", OrderType::LIMIT))); 

	book->displayOrder(); 


	FifoMatch match; 

	match.Match(book->getBids(), book->getAsks()); 

	book->displayOrder(); 
	book->getBestBidAsk(); 




	/*


	book->addOrder(*orderObjBuy); 
	book->displayOrder(*orderObjBuy);
	if (book->cancelOrder(orderId))
	{
		book->displayOrder(*orderObjBuy);
		book->getBestBidAsk();
	}
	else
	{
		book->modifyOrder(orderId, price, quantity);
		book->displayOrder(*orderObjBuy);
		book->getBestBidAsk();
	}

	*/
	


	return EXIT_SUCCESS; 
}
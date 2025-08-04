#include <iostream>
#include <nlohmann/json.hpp>
#include <fstream>
#include "book.h"




// sending bid/ask data to python for visualization 

using json = nlohmann::json;


void OrderBooktoFile(std::unique_ptr<OrderBook>& orderObject) {

	json ds; 
	
	// send bids 
	for (const auto& [price, orders] : orderObject->getBids())
	{
		int totalQty = 0;
		
		for (const auto& order : orders)
		{
			totalQty += order.getQuantity(); 

			
		}

		ds["bids"].push_back({ {"price", price}, {"quantity", totalQty} }); 

	}

	// send asks 
	
	
	for (const auto& [price, orders] : orderObject->getAsks())
	{
		int totalQty = 0;

		for (const auto& order : orders)
		{
			totalQty += order.getQuantity();


		}
		ds["asks"].push_back({ {"price", price}, {"quantity", totalQty} });

	}

	std::ofstream out("orderbook.json"); 

	if (!out)
	{
		std::cerr << "Failed to Write Orderbook to JSON!!!\n"; 
		return; 
	}

	out << ds.dump(4); 
	
}



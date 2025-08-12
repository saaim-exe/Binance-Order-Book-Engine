#include <iostream>
#include <nlohmann/json.hpp>
#include <fstream>

#include "book.h"
#include "Level.h"


using json = nlohmann::json;

// convnerting bid/ask data to json format for kafka stream  

std::string OrderBooktoJSON(const std::unique_ptr<OrderBook>& book, const std::unique_ptr<Level>& levels)
{
	json message;

	auto bidLevels = levels->getL2Bids(book);
	auto askLevels = levels->getL2Asks(book);

	for (const auto& level : bidLevels)
	{

		message["bids"].push_back({ {"price", level.price}, {"quantity", level.quantity} });

	}

	for (const auto& level : askLevels)
	{


		message["asks"].push_back({ {"price", level.price}, {"quantity", level.quantity} });

	}

	std::string payload = message.dump();


	return payload;
}
#include <iostream>
#include <nlohmann/json.hpp>
#include <fstream>
#include <shared_mutex>
#include <optional>

#include "book.h"
#include "Level.h"
#include "ticks.hpp"


using json = nlohmann::json;

// convnerting bid/ask data to json format for kafka stream  

std::string OrderBooktoJSON(const std::unique_ptr<OrderBook>& book, const std::unique_ptr<Level>& levels)
{
	json message;

	std::size_t topN = 5; // top 5 levels

	//	auto bidLevels = levels->getL2Bids(book, topN);
		//auto askLevels = levels->getL2Asks(book, topN);

	//bidL2 = levels->getL2Bids(book, topN);
		//askL2 = levels->getL2Asks(book, topN); 

	std::vector<bookLevels> bidL2, askL2; 
	std::map<Price, Quantity> bidDepthCopy, askDepthCopy; // copies for thread

	{
		std::shared_lock<std::shared_mutex> lk(book->mutex());

		const auto& bidsMap = book->getBids();
		const auto& asksMap = book->getAsks();

		bidL2.clear(); 
		askL2.clear(); 
		bidL2.reserve(topN); 
		askL2.reserve(topN); 
		bidDepthCopy.clear(); 
		askDepthCopy.clear(); 


		std::size_t countbids = 0; 
		std::size_t countasks = 0; 

		// depth copy insertion 
		for (const auto& [price, orders] : bidsMap)
		{
			Quantity total_qty = 0; 

			for (const auto& order : orders)
			{
				total_qty += order.getQuantity(); 
			}
			bidDepthCopy.emplace(price, total_qty); 

			if (countbids < topN)
			{
				bidL2.push_back({ price, total_qty });
				countbids++; 
			}

		}

		for (const auto& [price, orders] : asksMap)
		{
			Quantity total_qty = 0; 

			for (const auto& order : orders)
			{

				total_qty += order.getQuantity();
			}
			askDepthCopy.emplace(price, total_qty); 

			if (countasks < topN)
			{

				askL2.push_back({ price, total_qty });
				countasks++;
			}
		}


		// best bid / best ask/ spread /mid 

		if (!bidsMap.empty() && !asksMap.empty())
		{

			Price bb = bidsMap.begin()->first; 
			Price ba = asksMap.begin()->first;

			long long spread_ticks = (long long)ba - (long long)bb; 
			double spread_px = ticks_to_price(spread_ticks); 
			double mid_px = (ticks_to_price(bb) + ticks_to_price(ba)) * 0.5; 
				
			message["bestBid"] = ticks_to_price(bb);
			message["bestAsk"] = ticks_to_price(ba);
			message["spread"] = spread_px;
			message["mid"] = mid_px;



			// debug 
			std::cerr << "[TOP] bbT=" << bb << " baT=" << ba
				<< " | spread_ticks=" << spread_ticks
				<< " spread_px=" << spread_px << "\n";

		}

	}



	// levels 
	for (const auto& level : bidL2)
	{

		message["bids"].push_back({ {"price", ticks_to_price(level.price)}, {"quantity", ticks_to_qty(level.quantity)} });

	}

	for (const auto& level : askL2)
	{


		message["asks"].push_back({ {"price", ticks_to_price(level.price)}, {"quantity", ticks_to_qty(level.quantity)} });

	}


/*	if (!bidL2.empty() && !askL2.empty())
	{
		const auto& bestBid = bidL2.front(); 
		const auto& bestAsk = askL2.front(); 

		const double bid_px = ticks_to_price(bestBid.price);
		const double ask_px = ticks_to_price(bestAsk.price); 

		message["spread"] = ask_px - bid_px; 
		message["mid"] = (ask_px + bid_px) * 0.5; 

	}*/

	// depth 

	for (auto it = bidDepthCopy.rbegin(); it != bidDepthCopy.rend(); ++it)
	{
		message["bidDepth"].push_back({ {"price", ticks_to_price(it->first)}, {"quantity", ticks_to_qty(it->second)} });


	}
	for (auto it = askDepthCopy.begin(); it != askDepthCopy.end(); ++it)
	{
		message["askDepth"].push_back({ {"price", ticks_to_price(it->first)}, {"quantity", ticks_to_qty(it->second)} });


	}
	


	std::string payload = message.dump();


	return payload;
}


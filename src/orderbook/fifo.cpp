#include "fifo.h"
#include "book.h"
#include "order.h"


void FifoMatch::Match(std::map<Price, std::vector<Order>, std::greater<Price>>& bids, std::map<Price, std::vector<Order>>& asks)
{

	bool matchedOnce = false; // bool state to check for if no matches in tick 

	while (!bids.empty() && !asks.empty())
	{

		bool canMatch = false;



		auto& buyVec = bids.begin()->second;
		auto& askVec = asks.begin()->second;

		auto& buyOrder = buyVec.front();
		auto& askOrder = askVec.front();




		Quantity matchQty = std::min(buyOrder.getQuantity(), askOrder.getQuantity());
		Price tradePrice = askOrder.getPrice();

		if (buyOrder.getOrderType() == OrderType::MARKET && askOrder.getOrderType() == OrderType::MARKET)
		{

			std::cerr << "INVALID::BOTH ORDERS ARE MARKET::\n";

			break;
		}
		if (buyOrder.getOrderType() == OrderType::MARKET && askOrder.getOrderType() == OrderType::LIMIT)
		{


			canMatch = true;
			tradePrice = askOrder.getPrice();


		}
		if (buyOrder.getOrderType() == OrderType::LIMIT && askOrder.getOrderType() == OrderType::MARKET)
		{
			canMatch = true;
			tradePrice = buyOrder.getPrice();


		}
		if (buyOrder.getOrderType() == OrderType::LIMIT && askOrder.getOrderType() == OrderType::LIMIT)
		{
			if (buyOrder.getPrice() >= askOrder.getPrice())
			{
				canMatch = true;

				tradePrice = askOrder.getPrice();

			}
			else
			{
				break;
			}


		}

		if (canMatch)
		{
			matchedOnce = true;

			//subtracting matched qty from both orders 

			const Quantity remainingBidQty = buyOrder.getQuantity() - matchQty; // 0 
			const Quantity remainingAskQty = askOrder.getQuantity() - matchQty; // 5 


			std::cout << "Matched " << matchQty << " units @ " << tradePrice << '\n';


			buyOrder.setQuantity(remainingBidQty);
			askOrder.setQuantity(remainingAskQty);


			// if any order qty < 0 , remove from book 


			if (buyOrder.getQuantity() == 0)
			{

				buyVec.erase(buyVec.begin());

			}

			if (askOrder.getQuantity() == 0)
			{

				askVec.erase(askVec.begin());

			}

			if (buyVec.empty()) {

				bids.erase(bids.begin());

			}
			if (askVec.empty())
			{

				asks.erase(asks.begin());

			}


			tradelog.push_back({ buyOrder.getOrderId(), askOrder.getOrderId(), tradePrice, matchQty, askOrder.getTimestamp() });

		}

	}


	if (!matchedOnce)
	{

		std::cout << "No Matches this Tick " << std::endl;

	}


}

void FifoMatch::displayTrades() const {

	if (tradelog.empty())
	{
		std::cout << "--- TRADE LOG EMPTY ---\n";
		return;
	}


	std::cout << "---TRADE LOG---\n";

	for (auto& trade : tradelog)
	{
		std::cout << "BID ORDER: " << trade.bidID
			<< " matched with ASK ORDER: " << trade.askID
			<< " | Price: " << trade.price
			<< " | Qty: " << trade.quantity
			<< " | Timestamp: " << trade.timestamp << '\n';
	}

}


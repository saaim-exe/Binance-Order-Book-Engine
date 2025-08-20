#pragma once
#include "order.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <utility>
#include <queue>
#include <map>
#include <unordered_map>
#include <cstdlib>
#include <ctime>
#include <mutex>
#include <shared_mutex>


class OrderBook {

public:

	OrderBook() = default;

	void addOrder(std::unique_ptr<Order> orderObject); // FOR L3 STREAM 

	void addOrder(std::unique_ptr<Order> orderObject, bool aggregate); // FOR L2 STREAM 

	void applyL2Deltas(Side side, Price p, Quantity  q); 

	void eraseLevel(Side side, Price p); 

	void applyPartialDepthSnapshot(const std::unordered_map<Price, Quantity>& bidLevels, const std::unordered_map<Price, Quantity>& askLevels); 

	//void beginPartialDepthSnapshot();
	// void endPartialDepthSnapshot();

	void displayOrder() const;
	bool cancelOrder(OrderID& id);
	bool modifyOrder(OrderID& id, Price& price, Quantity& quantity);
	void displayBestBidAsk();

	//	Quantity getMarketDepth(); 

	OrderID generateID() {

		std::lock_guard<std::shared_mutex> lock(bookMutex);

		return nextID++;
	} // random id generation 

	std::unique_ptr<Order> makeOrder(const OrderID& id, const Price& price, const Quantity& quantity, const Side& side, const Timestamp& timestamp, const OrderType& type);


	// const/non-cost accessor functions for get bids/asks
	std::map<Price, std::vector<Order>, std::greater<Price>>& getBids();
	const std::map<Price, std::vector<Order>, std::greater<Price>>& getBids() const;

	std::map<Price, std::vector<Order>>& getAsks();
	const std::map<Price, std::vector<Order>>& getAsks() const;

	Price getBestBid() const {
		if (!bids.empty())
		{
			return bids.begin()->first;
		}
	}
	Price getBestAsk() const {
		if (!asks.empty())
		{
			return asks.begin()->first;

		}
	}

	const std::unordered_map<OrderID, Order>& getOrderIndex() const;

	std::shared_mutex& mutex() const { return bookMutex; }

	OrderID getBidIDs;
	OrderID getAskIDs;

private:


	std::map<Price, std::vector<Order>, std::greater<Price>> bids;
	std::map<Price, std::vector<Order>> asks;

	std::unordered_map <OrderID, Order> orderIndex;

	Price bb; // best bid 
	Price ba;  // best ask 

	OrderID nextID = 1;

	mutable std::shared_mutex bookMutex; // access 


	// partial-depth snapshots 
	uint64_t depthEpoch{ 0 }; 
	std::unordered_map<Price, Quantity> seenBidEpoch; 
	std::unordered_map<Price, Quantity> seenAskEpoch; 

};

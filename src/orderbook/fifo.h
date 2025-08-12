#pragma once
#include "order.h"
#include "book.h"
#include <iostream>
#include <vector>
#include <algorithm>

struct Trade {
	OrderID bidID;
	OrderID askID;
	Price price;
	Quantity quantity;
	Timestamp timestamp;
};


class FifoMatch {


public:

	void Match(std::map<Price, std::vector<Order>, std::greater<Price>>& bids, std::map<Price, std::vector<Order>>& asks);
	// std::string getCurrentTimestamp() const;
	void displayTrades() const;

private:
	std::vector<Trade> tradelog;

};


#pragma once
#include <iostream>
#include "book.h"
#include "order.h"
#include "ticks.hpp"

// retrieves L1 , L2 , L3 data from orderbook 




struct bookLevels {

	Price price;
	Quantity quantity;
};


struct L3Data {

	OrderID orderId;
	Price price;
	Quantity quantity;
	Timestamp timestamp;

};


class Level {

public:


	bookLevels getL1Bids(const std::unique_ptr<OrderBook>& book);
	bookLevels getL1Asks(const std::unique_ptr<OrderBook>& book);

	std::vector<bookLevels> getL2Bids(const std::unique_ptr<OrderBook>& book, std::size_t topN); // returns top n  price levels + volume 
	std::vector<bookLevels> getL2Asks(const std::unique_ptr<OrderBook>& book, std::size_t topN);// returns top n price levels + volume 

	std::vector<L3Data> getL3Bids(const std::unique_ptr<OrderBook>& book); // full orderbook data 
	std::vector <L3Data> getL3Asks(const std::unique_ptr<OrderBook>& book); // full orderbook data

	const std::map<Price, Quantity>& getBidDepth() const {
		return bidDepth;
	}

	const std::map<Price, Quantity>& getAskDepth() const {

		return askDepth;
	}



	// std::vector<Price> spread(const std::unique_ptr<OrderBook>& book); 


private:


	std::map<Price, Quantity> bidDepth;
	std::map<Price, Quantity> askDepth;

	

	Price bestBid; 
	Price bestAsk; 



};
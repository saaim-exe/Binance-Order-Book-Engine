#pragma once
#include <iostream>
#include "book.h"
#include "order.h"

// retrieves L1 , L2 data from orderbook 

class Level : OrderBook {

public: 
	void L1(); 
	void L2();
	void L3(); 



private: 

	Price bestBid = getBestBid(); 
	Price bestAsk = getBestAsk(); 


};
#include "Level.h"

bookLevels Level::getL1Bids(const std::unique_ptr<OrderBook>& book)
{

	Quantity totalQty = 0;
	auto bestBid = book->getBestBid();
	this->bestBid = bestBid; 


	for (const auto& [id, order] : book->getOrderIndex())
	{



		if (order.getPrice() == bestBid && order.getSide() == Side::BUY)
		{

			totalQty += order.getQuantity();

		}

	}
	return { bestBid,  totalQty };

}

bookLevels Level::getL1Asks(const std::unique_ptr<OrderBook>& book)
{

	Quantity totalQty = 0;
	auto bestAsk = book->getBestAsk();
	this->bestAsk = bestAsk; 

	for (const auto& [id, order] : book->getOrderIndex())
	{


		if (order.getPrice() == bestAsk && order.getSide() == Side::SELL)
		{

			totalQty += order.getQuantity();

		}

	}



	return { bestAsk,  totalQty };

}


std::vector<bookLevels> Level::getL2Bids(const std::unique_ptr<OrderBook>& book, std::size_t topN)
{

	std::vector<bookLevels> levels;
	levels.reserve(topN); 

	const auto& bids = book->getBids();

	std::size_t count = 0;

	for (const auto& [price, orders] : bids)
	{
		if (count >= topN)
			break;

		Quantity totalQty = 0;

		for (const auto& order : orders)
		{

			totalQty += order.getQuantity();

		}

		levels.push_back({ price, totalQty });



		count++;
	}


	return levels;
}

std::vector<bookLevels> Level::getL2Asks(const std::unique_ptr<OrderBook>& book, std::size_t topN)
{

	std::vector<bookLevels> levels;
	levels.reserve(topN); 

	const auto& asks = book->getAsks();

	std::size_t count = 0;

	for (const auto& [price, orders] : asks)
	{
		if (count >= topN)
		{
			break;
		}

		Quantity totalQty = 0;

		for (const auto& order : orders)
		{

			totalQty += order.getQuantity();

		}

		levels.push_back({ price, totalQty });



		count++;
	}


	return levels;
}


std::vector<L3Data> Level::getL3Bids(const std::unique_ptr<OrderBook>& book) {


	std::vector<L3Data> levels;
	std::map<Price, Quantity> localdepth;



	const auto& bids = book->getBids();



	for (const auto& [price, orders] : bids)
	{

		for (const auto& order : orders)
		{

			L3Data data{

				order.getOrderId(),
				order.getPrice(),
				order.getQuantity(),
				order.getTimestamp()
			};



			levels.push_back(data);
			localdepth[order.getPrice()] += order.getQuantity();

		}



	}

	this->bidDepth = localdepth;



	return levels;
}

std::vector<L3Data> Level::getL3Asks(const std::unique_ptr<OrderBook>& book) {


	std::vector<L3Data> levels;
	std::map<Price, Quantity> localdepth;



	const auto& asks = book->getAsks();



	for (const auto& [price, orders] : asks)
	{


		for (const auto& order : orders)
		{

			L3Data data{

				order.getOrderId(),
				order.getPrice(),
				order.getQuantity(),
				order.getTimestamp()
			};

			levels.push_back(data);
			localdepth[order.getPrice()] += order.getQuantity();

		}



	}


	this->askDepth = localdepth;


	return levels;

}



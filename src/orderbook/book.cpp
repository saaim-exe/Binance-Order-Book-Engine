#include "book.h"
#include "ticks.hpp"

#define TRUE 1
#define FALSE 0 


void OrderBook::addOrder(std::unique_ptr<Order> orderObject)
{
	std::unique_lock<std::shared_mutex> lock(bookMutex); 


	const Order& order = *orderObject;

	if (order.getQuantity() == 0)  return;


	if (order.getSide() == Side::BUY)
	{


		bids[order.getPrice()].emplace_back(order);
		orderIndex[order.getOrderId()] = order;

		getBidIDs = order.getOrderId();

	}
	else if (order.getSide() == Side::SELL)
	{

		asks[order.getPrice()].emplace_back(order);
		orderIndex[order.getOrderId()] = order;

		getAskIDs = order.getOrderId();

	}

	/* debug 
	std::cerr << "[WRITE] after " << (order.getSide() == Side::BUY ? "BUY" : "SELL")
		<< " p=" << order.getPrice() << " q=" << order.getQuantity()
		<< "  bids=" << bids.size() << " asks=" << asks.size() << "\n";*/



}

void OrderBook::addOrder(std::unique_ptr<Order> orderObject, bool aggregate)
{
	

	if (!aggregate) { 
		
		this->addOrder(std::move(orderObject));
		return; 

	
	}

	// L2 

	std::unique_lock<std::shared_mutex> lk(bookMutex); 

	const Order& order = *orderObject;
	
	if (order.getQuantity() == 0)
	{
		if (order.getSide() == Side::BUY)
		{
			bids.erase(order.getPrice());
			seenBidEpoch.erase(order.getPrice()); 
		}
		if (order.getSide() == Side::SELL)
		{
			asks.erase(order.getPrice());
			seenAskEpoch.erase(order.getPrice()); 
		}
		return; 
	}

	if (order.getSide() == Side::BUY)
	{
		auto& v = bids[order.getPrice()]; 
		v.clear(); 
		v.emplace_back(order); 
		seenBidEpoch[order.getPrice()] = depthEpoch; 
	}
	else if (order.getSide() == Side::SELL)
	{
		auto& v = asks[order.getPrice()];
		v.clear(); 
		v.emplace_back(order);
		seenAskEpoch[order.getPrice()] = depthEpoch; 
	}


}

void OrderBook::applyL2Deltas(Side side, Price p, Quantity q) {

	const OrderID id = generateID(); 
	
	std::unique_lock<std::shared_mutex> lk(bookMutex);


	/* debug 
	std::cerr << "[DELTA] " << (side == Side::BUY ? 'B' : 'A')
		<< " p=" << ticks_to_price(p)
		<< " q=" << ticks_to_qty(q) << "\n";
		*/
	 

	if (side == Side::BUY) {
		if (q == 0) {
			if (auto it = bids.find(p); it != bids.end())
			{
				bids.erase(it); 
			}
			// return;
		}
		else {

			auto& v = bids[p];
			v.clear();
			v.emplace_back(id, p, q, Side::BUY, "0", OrderType::LIMIT);

		}		
		seenBidEpoch[p] = depthEpoch; 
		return; 
	}
	
	// SELL
	if (q == 0) {

		if (auto it = asks.find(p); it != asks.end())
		{
			asks.erase(it); 
		}

		// return;
		
	}
	else {

		auto& v = asks[p];
		v.clear();
		v.emplace_back(id, p, q, Side::SELL, "0", OrderType::LIMIT);
		
	}
	seenAskEpoch[p] = depthEpoch;

		
}

void OrderBook::applyPartialDepthSnapshot(const std::unordered_map<Price, Quantity>& bidLvls, const std::unordered_map<Price, Quantity>& askLvls)
{

	const OrderID id = generateID();


	std::unique_lock<std::shared_mutex> lk(bookMutex);

	++depthEpoch; 
	seenBidEpoch.clear(); 
	seenAskEpoch.clear(); 

	// BIDS 
	for (const auto& [price, qty] : bidLvls)
	{
		if (qty == 0)
		{

			bids.erase(price); 
			continue; 
		}


		auto& v = bids[price];
		v.clear(); 
		v.emplace_back(id, price, qty, Side::BUY, "0", OrderType::LIMIT);
		seenBidEpoch[price] = depthEpoch; 
	}

	for (auto it = bids.begin(); it != bids.end();)
	{
		 
		auto found = seenBidEpoch.find(it->first);
		if (found == seenBidEpoch.end() || found->second != depthEpoch)
		{
			it = bids.erase(it);
		}
		else
		{
			++it;
		}

	}

	// ASKS 

	for (const auto& [price, qty] : askLvls)
	{
		if (qty == 0)
		{

			asks.erase(price); 
			continue;
		}



		auto& v = asks[price];
		v.clear(); 
		v.emplace_back(id, price, qty, Side::SELL, "0", OrderType::LIMIT);
		seenAskEpoch[price] = depthEpoch; 
	}

	for (auto it = asks.begin(); it != asks.end();)
	{
		auto found = seenAskEpoch.find(it->first);
		if (found == seenAskEpoch.end() || found->second != depthEpoch)
		{
			it = asks.erase(it);
		}
		else
		{
			++it;
		}
	}

}

/*void OrderBook::beginPartialDepthSnapshot() {
	std::unique_lock<std::shared_mutex> lk(bookMutex);

	++depthEpoch; 
}

void OrderBook::endPartialDepthSnapshot()
{
	std::unique_lock<std::shared_mutex> lk(bookMutex);

	// BIDS 
	for (auto it = bids.begin(); it != bids.end();)
	{
		auto found = seenBidEpoch.find(it->first);
		if (found == seenBidEpoch.end() || found->second != depthEpoch)
		{
			it = bids.erase(it); 
		}
		else
		{
			++it; 
		}

	}

	// ASKS
	for (auto it = asks.begin(); it != asks.end();)
	{
		auto found = seenAskEpoch.find(it->first); 
		if (found == seenAskEpoch.end() || found->second != depthEpoch)
		{
			it = asks.erase(it); 
		}
		else
		{
			++it; 
		}



	}



}*/

void OrderBook::eraseLevel(Side side, Price p)
{
	std::unique_lock<std::shared_mutex> lk(bookMutex);

	if (side == Side::BUY)
	{
		bids.erase(p);
	}
	else
	{
		asks.erase(p); 
	}
}


void OrderBook::displayOrder() const
{
	std::unique_lock<std::shared_mutex> lock(bookMutex); 


	std::cout << "\n--- ORDERBOOK ---\n";
	std::cout << "\nBIDS:\n";

	for (const auto& [price, orders] : bids)
	{

		for (const auto& order : orders)
		{
			std::cout << "Order ID: " << order.getOrderId();
			std::cout << " | Price: " << std::setprecision(2) << order.getPrice();
			std::cout << " | Quantity: " << std::setprecision(2) << order.getQuantity();
			std::cout << " | Timestamp: " << order.getTimestamp() << '\n';

		}
	}

	std::cout << "\nASKS: \n";

	for (const auto& [price, orders] : asks)
	{

		for (const auto& order : orders)
		{
			std::cout << "Order ID: " << order.getOrderId();
			std::cout << " | Price: " << std::setprecision(2) << order.getPrice();
			std::cout << " | Quantity: " << std::setprecision(2) << order.getQuantity();
			std::cout << " | Timestamp: " << order.getTimestamp() << '\n';

		}
	}

	std::cout << "-------------------\n";


}


bool OrderBook::cancelOrder(OrderID& id) {

	// if they wawnt to cancel order we find the location which there id is at with their price and quantity and 
	// then remove that order 

	std::unique_lock<std::shared_mutex> lock(bookMutex); 


	std::cout << "CANCEL ORDER? (1 = YES, 2 = NO)\n";
	int user_input;
	std::cin >> user_input;

	if (user_input == TRUE)
	{

		std::cout << "Please Enter OrderID: ";
		std::cin >> id;


		auto it = orderIndex.find(id);

		if (it != orderIndex.end())
		{

			Order order = it->second;
			Side side = it->second.getSide();
			Price oldPrice = it->second.getPrice();


			std::vector<Order>* OrderListPtr = nullptr;

			if (side == Side::BUY)
			{

				OrderListPtr = &bids[oldPrice];

			}
			else if (side == Side::SELL)
			{

				OrderListPtr = &asks[oldPrice];

			}


			if (OrderListPtr)

			{

				auto& orderList = *OrderListPtr;

				// iterating throuhg all prices and removing the one with id match 
				for (auto orderIt = orderList.begin(); orderIt != orderList.end(); orderIt++)
				{
					if (orderIt->getOrderId() == id)
					{
						orderList.erase(orderIt);

						if (orderList.empty())
						{
							if (side == Side::BUY)
							{
								bids.erase(oldPrice);

							}
							else if (side == Side::SELL)
							{

								asks.erase(oldPrice);
							}
						}

						break;
					}


				}

			}

			orderIndex.erase(id);
			return true;
		}
		else {
			std::cout << "ORDER ID Not Found \n";
			return false;
		}



	}
	else if (user_input == FALSE)
	{


		return false;

	}
	return false;
}


bool OrderBook::modifyOrder(OrderID& id, Price& price, Quantity& quantity)
{
	// allow user to modify the existing state of thier order i.e. change the price / quantity.
	// price and quantity cannot be negative

	std::unique_lock<std::shared_mutex> lock(bookMutex);


	std::cout << "Would you like to modify order? (1 = YES, 2 = NO)\n";
	int user_input;
	std::cin >> user_input;

	if (user_input == TRUE)
	{
		std::cout << "Please Enter OrderID: ";
		std::cin >> id;
		std::cout << '\n';

		std::cout << "SELECT: [1. PRICE] [2. QUANTITY]" << '\n';
		int selection;
		std::cin >> selection;

		if (selection == 1)
		{
			std::cout << "PRICE[CURRENT]: " << price << '\n';
			std::cout << "PRICE[UPDATE]: ";
			std::cin >> price;

			if (price < 0)
			{
				std::cout << "NEGATIVE PRICE ERROR '\n";
				return false;
			}


			std::cout << '\n';

		}
		else if (selection == 2)
		{

			std::cout << "QUANTITY[CURRENT]: " << quantity << '\n';
			std::cout << "QUANTITY[UPDATE]: ";
			std::cin >> quantity;

			if (quantity < 0)
			{
				std::cout << "NEGATIVE QUANTITY ERROR '\n";
				return false;
			}
			std::cout << '\n';
		}
		else
		{
			std::cout << "INVALID INPUT!";
			return false;
		}





		auto it = orderIndex.find(id);

		if (it != orderIndex.end())
		{
			Side side = it->second.getSide();
			Price oldPrice = it->second.getPrice();
			Quantity oldQuantity = it->second.getQuantity();

			std::vector<Order>* OrderListPtr = nullptr;

			if (side == Side::BUY)
			{

				OrderListPtr = &bids[oldPrice];


			}
			else if (side == Side::SELL)
			{
				OrderListPtr = &asks[oldPrice];

			}

			auto modifiedOrder = std::make_unique<Order>(id, price, quantity, Side::NONE, "updated", OrderType::NONE);

			if (OrderListPtr) {


				auto& orderList = *OrderListPtr;

				for (auto orderIt = orderList.begin(); orderIt != orderList.end(); orderIt++)
				{

					// we are now checking each order from either bid or ask 
					// remove order from current postion 
					// update with new price 
					// re-insert the updated order with addOrder() 

					if (orderIt->getOrderId() == id)
					{
						orderList.erase(orderIt);

						addOrder(std::move(modifiedOrder));


						if (orderList.empty())
						{
							if (side == Side::BUY)
							{
								bids.erase(oldPrice);

							}
							else if (side == Side::SELL)
							{

								asks.erase(oldPrice);
							}
						}

						break;
					}



				}



			}


		}
		else
		{
			std::cout << "ORDER ID Not Found \n";
			return false;
		}

	}
	else if (user_input == FALSE)
	{
		return false;

	}

	return false;
}


void OrderBook::displayBestBidAsk()
{

	// from list of current bid / asks find highest bid and lowest ask , display 

	if (!bids.empty())
	{

		auto bestBid = bids.begin();
		auto bb = bestBid->first;
		this->bb = bb;

		for (const auto& [id, order] : orderIndex)
		{

			if (order.getPrice() == bb && order.getSide() == Side::BUY)
			{

				auto bestqty = order.getQuantity();


				std::cout << "Best Bid [Price/Qty]: " << "[ " << bb << "/" << bestqty << " ]" << '\n';
				break;

			}


		}


	}
	else {

		std::cout << "Best Bid [Price/Qty]: " << "[ " << FALSE << "/" << FALSE << " ]" << '\n';

	}
	if (!asks.empty())
	{

		auto bestAsk = asks.begin();
		auto ba = bestAsk->first;
		this->ba = ba;

		for (const auto& [id, order] : orderIndex)
		{

			if (order.getPrice() == ba && order.getSide() == Side::SELL)
			{

				auto bestqty = order.getQuantity();

				std::cout << "Best Ask [Price/Qty]: " << "[ " << ba << "/" << bestqty << " ]" << '\n';
				break;

			}
		}

	}
	else
	{
		std::cout << "Best Ask [Price/Qty]: " << "[ " << FALSE << "/" << FALSE << " ]" << '\n';

	}

}


std::unique_ptr<Order> OrderBook::makeOrder(const OrderID& id, const Price& price, const Quantity& quantity, const Side& side, const Timestamp& timestamp, const OrderType& type)
{
	return std::make_unique<Order>(id, price, quantity, side, timestamp, type);
}



std::map<Price, std::vector<Order>, std::greater<Price>>& OrderBook::getBids()
{

	return bids;
}

std::map<Price, std::vector<Order>>& OrderBook::getAsks() {
	return asks;
};


const std::map<Price, std::vector<Order>, std::greater<Price>>& OrderBook::getBids() const
{
	return bids;
};

const std::map<Price, std::vector<Order>>& OrderBook::getAsks() const {
	return asks;
};


const std::unordered_map<OrderID, Order>& OrderBook::getOrderIndex() const {

	return orderIndex;
}
#include "book.h"

#define TRUE 1
#define FALSE 0 


void OrderBook::addOrder(std::unique_ptr<Order> orderObject)
{

	std::lock_guard<std::mutex> lock(bookMutex); // safe thread access

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
}

void OrderBook::displayOrder() const
{

	std::lock_guard<std::mutex> lock(bookMutex); // safe thread access


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
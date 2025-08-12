#pragma once
#include <iostream>
#include <cstdint>
#include <algorithm>
#include <cstddef>
#include <map>
#include <unordered_map>
#include <string>
#include <sstream>

enum class Side {
	NONE,
	BUY,
	SELL
};


enum class OrderType {
	NONE,
	LIMIT,
	MARKET,
	STOP
};


// aliases
typedef std::uint64_t OrderID;
typedef std::uint32_t Price;
typedef std::uint32_t Quantity;
typedef std::string Timestamp;

struct OrderInfo {
	OrderID orderID{};
	Price price{};
	Quantity quantity{};
	Timestamp timestamp{};
	Side side;

};

class Order {

public:

	Order() {

		orderID = 0;
		price = 0;
		quantity = 0;
		timestamp = "";
	};



	Order(OrderID orderID, Price price, Quantity quantity, Side side, Timestamp timestamp, OrderType type)
		: orderID(orderID), price(price), quantity(quantity), side(side), timestamp(timestamp), type(type) {
	}

	//accessor methods
	OrderID getOrderId() const { return orderID; }
	Price getPrice() const { return price; }
	Quantity getQuantity() const { return quantity; }
	Side getSide() const { return side; }
	Timestamp getTimestamp() const { return timestamp; }
	OrderType getOrderType() const { return type; }



	void setQuantity(Quantity quantity);
	//std::string serializeOrder(); 
	// static Order* deserializeOrder(std::string str); 



private:

	OrderID orderID;
	Price price;
	Quantity quantity;
	Side side;
	Timestamp timestamp;
	OrderType type;


};

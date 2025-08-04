#include "order.h"


void Order::setQuantity(Quantity quantity)
{
	this->quantity = quantity; 

}

/*std::string Order::serializeOrder()
{
	std::ostringstream oss;
	oss << "Order: "
		<< this->orderID << ", "
		<< this->price << ", "
		<< this->quantity << ", "
		<< static_cast<int>(type); 


	return oss.str(); 
}
*/

/*Order* Order::deserializeOrder(std::string str)
{







}
*/







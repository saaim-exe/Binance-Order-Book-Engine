#include "binanceapi.hpp"

using namespace Client; 

namespace Client {

    binapi::ws::part_depths_t o;

    boost::asio::io_context ioctx;
    
    binapi::ws::websockets ws{
        ioctx
        , "stream.binance.com"
        , "9443"
    };

}


void Client::getPartialDepth(std::map<double, double>& bids, std::map<double, double>& asks, std::unique_ptr<OrderBook>& book, std::unique_ptr<FifoMatch>& match)
{
    ws.part_depth("BTCUSDT", binapi::e_levels::_5, binapi::e_freq::_100ms,
        [&bids, &asks, &book, &match](const char* fl, int ec, std::string emsg, const binapi::ws::part_depths_t& depths) {
            if (ec) {
                std::cerr << "subscribe part_depth error: fl=" << fl << ", ec=" << ec << ", emsg=" << emsg << std::endl;

                return false;
            }

            // std::cout << "part_depths: " << depths << std::endl;

            bids.clear(); 
            asks.clear(); 

            for (const auto& [price, qty] : depths.b)
            {

                if (qty == 0.0)
                    continue;

                
                bids[static_cast<double>(price)] = static_cast<double>(qty);
                auto order = deserialize(*book, static_cast<double>(price), static_cast<double>(qty), Side::BUY); 
                book->addOrder(std::move(order));

            }
            for (const auto& [price, qty] : depths.a)
            {
                if (qty == 0.0)
                    continue;

                asks[static_cast<double>(price)] = static_cast<double>(qty); 
                auto order = deserialize(*book, static_cast<double>(price), static_cast<double>(qty), Side::SELL);
                book->addOrder(std::move(order));
            }

            match->Match(book->getBids(), book->getAsks());

            return true;

        }
    );

   

   
}

std::unique_ptr<Order> Client::deserialize(OrderBook& book, double price, double quantity, Side side) {

    OrderID id = book.generateID(); 
    Price p = price;
    Quantity q = quantity; 
    Timestamp t = "0"; 
    OrderType type = OrderType::LIMIT; 


    return book.makeOrder(id, p, q, side, t, type); 
}


/*void BinanceAPI::on_open(websocketpp::connection_hdl hdl, client* c)
{
    std::cout << "WebSocket connection opened!" << std::endl;

    websocketpp::lib::error_code ec;

    client::connection_ptr con = c->get_con_from_hdl(hdl, ec);
    if (ec) {
        std::cout << "Failed to get connection pointer: " << ec.message() << std::endl;
        return;
    }
    std::string payload = "{\"userKey\":\"API_KEY\", \"symbol\":\"EURUSD,GBPUSD\"}";
    c->send(con, payload, websocketpp::frame::opcode::text);
}


void BinanceAPI::on_message(websocketpp::connection_hdl, client::message_ptr msg)
{

    std::cout << "Received Message" << msg->get_payload() << std::endl; 
}

void BinanceAPI::on_fail(websocketpp::connection_hdl hdl)
{
    std::cout << "Websocket connection Failed!" << std::endl; 
}

void BinanceAPI::on_close(websocketpp::connection_hdl hdl)
{

    std::cout << "Websocket connection closed!" << std::endl; 




}
*/
 




#include "binanceapi.hpp"
#include <shared_mutex>
#include <unordered_map>

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

    ws.part_depth("BTCUSDT", binapi::e_levels::_20, binapi::e_freq::_100ms,
        [&bids, &asks, &book, &match](const char* fl, int ec, std::string emsg, const binapi::ws::part_depths_t& depths) -> bool {

            try {

                if (ec) {
                    std::cerr << "subscribe part_depth error: fl=" << fl << ", ec=" << ec << ", emsg=" << emsg << std::endl;

                    return false;
                }


                // std::cout << "part_depths: " << depths << std::endl;

            //    bids.clear();
            //    asks.clear();


               

                std::unordered_map<Price, Quantity> mb, ma;
                std::map<Price, std::vector<Order>, std::greater<Price>> matchbids; 
                std::map<Price, std::vector<Order>> matchasks; 

                for (const auto& [px, qx] : depths.b)
                {
                    Price p = bid_price_to_ticks(px);
                    Quantity q = qty_to_ticks(qx); 
                    mb[p] = q; 

                    /* debug 
                    std::cerr << "[WS] B raw px=" << px
                        << " qx=" << qx
                        << " -> p=" << p
                        << " q=" << q << "\n";*/
                
                     // auto order = book->makeOrder(book->generateID(), price_to_ticks(px), qty_to_ticks(qx), Side::BUY, "0", OrderType::LIMIT);
                       //book->addOrder(std::move(order), true);

                }

                for (const auto& [p, q] : mb)
                {
                    if (q == 0)
                    {
                        book->eraseLevel(Side::BUY, p);
                    }
                    else
                    {
                        book->applyL2Deltas(Side::BUY, p, q); 
                    }

                }


                for (const auto& [px, qx] : depths.a)
                {
                    
                    Price p = ask_price_to_ticks(px);
                    Quantity q = qty_to_ticks(qx);
                    ma[p] = q;

                   // auto order = book->makeOrder(book->generateID(), price_to_ticks(px), qty_to_ticks(qx), Side::SELL, "0", OrderType::LIMIT);
                   //  book->addOrder(std::move(order), true);

                }

                for (const auto& [p, q] : ma)
                {
                    if (q == 0)
                    {
                        book->eraseLevel(Side::SELL, p); 
                    }
                    else
                    {
                        book->applyL2Deltas(Side::SELL, p, q); 
                    }

                }

                std::cerr << "[WS] mb.size=" << mb.size()
                    << "  ma.size=" << ma.size() << "\n";


                book->applyPartialDepthSnapshot(mb, ma); 
                // match->Match(mb, ma); 

                {
                    std::shared_lock lk(book->mutex());

                    // book debug 
                    std::cerr << "[BOOK] bids=" << book->getBids().size()
                        << " asks=" << book->getAsks().size() << "\n";

                    // 



                }

                return true;
            }
            catch (const std::exception& e) {
                std::cerr << "part_depth callback exception: " << e.what() << "\n";
                return true;
            }
            catch (...) {
                std::cerr << "part_depth callback unknown exception\n";
                return true;
            }
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

// book->beginPartialDepthSnapshot();
//          book->endPartialDepthSnapshot();
/*   for (const auto& [price, qty] : depths.b)
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

              // match->Match(book->getBids(), book->getAsks());

              return true;

          } */
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstddef>
#include <map>
#include <unordered_map>
#include <thread>
#include <string>
#include "order.h"
#include "book.h"
#include "fifo.h"
#include "binanceapi.hpp"
#include "Level.h"
#include "jsonconverter.hpp"
#include <librdkafka/rdkafkacpp.h>
#include "kafka.cpp"


static volatile sig_atomic_t run = 1;

static void sigterm(int sig) {
	run = 0;
}


using namespace Client; 

int main() {

	std::map<double, double> bids;
	std::map<double, double> asks; 

	auto book = std::make_unique<OrderBook>(); 
	auto match = std::make_unique<FifoMatch>();
	auto levels = std::make_unique<Level>(); 
	
	Client::getPartialDepth(bids, asks, book, match);


	std::thread KafkaThread([&]() {

	signal(SIGINT, sigterm);
	signal(SIGTERM, sigterm);



	std::string broker = "localhost:9802";
	std::string topic_name = "orderbook";
	std::string errstr;

	RdKafka::Conf* conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);

	DeliveryReport dr_cb;

	if (conf->set("dr_cb", &dr_cb, errstr) != RdKafka::Conf::CONF_OK) {

		std::cerr << errstr << std::endl;
		exit(1);
	}

	RdKafka::Producer* producer = RdKafka::Producer::create(conf, errstr);

	if (!producer)
	{

		std::cerr << "FAILED TO CREATE PRODUCER: " << errstr << std::endl;
		exit(1);
	}

	delete conf;

	while (run)
	{

		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		std::string payload = OrderBooktoJSON(book, levels);

		// quick debug to check if orderbook data is sent 

		std::cout << "Payload: " << payload << std::endl;

	retry:
		RdKafka::ErrorCode err = producer->produce(

			topic_name,

			RdKafka::Topic::PARTITION_UA,

			RdKafka::Producer::RK_MSG_COPY,

			const_cast<char*>(payload.c_str()), payload.size(),

			NULL, 0,

			0,

			NULL,

			NULL);

		if (err != RdKafka::ERR_NO_ERROR) {
			std::cerr << "% Failed to produce to topic " << topic_name << ": "
				<< RdKafka::err2str(err) << std::endl;

			if (err == RdKafka::ERR__QUEUE_FULL) {

				producer->poll(1000);
				goto retry;
			}

		}
		else {
			std::cerr << "% Enqueued message (" << payload.size() << " bytes) "
				<< "for topic " << topic_name << std::endl;
		}

		producer->poll(0);
	}
		producer->flush(10 * 1000);
		delete producer;
	}); 


	try {
		
		std::thread displayThread([&book]() {


			while (true)
			{
				std::this_thread::sleep_for(std::chrono::seconds(1));
				system("cls");
				// book->displayOrder();
				// OrderBooktoFile(book);
			}

			});

		Client::ioctx.run();
		displayThread.join();
		
	}
	catch (const std::exception& ex)
	{

		std::cerr << "std::exception: " << ex.what() << std::endl;
	}

	KafkaThread.join();

	

	return EXIT_SUCCESS; 
}
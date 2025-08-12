#include "kafka.h"


void DeliveryReport::dr_cb(RdKafka::Message& message)
{
	if (message.err()) {
		std::cerr << "Message delivery failed: " << message.errstr()
			<< std::endl;
	}


	else {
		std::cerr << " % Message delivered to topic " << message.topic_name()
			<< " [" << message.partition() << " ] at offset "
			<< message.offset() << std::endl;
	}
}
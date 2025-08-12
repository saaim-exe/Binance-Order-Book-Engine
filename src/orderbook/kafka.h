#pragma once
#include <iostream>
#include <librdkafka/rdkafkacpp.h>
#include "book.h"


// producer 

class DeliveryReport : public RdKafka::DeliveryReportCb {

public:

	void dr_cb(RdKafka::Message& message) override;


};
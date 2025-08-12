import json, time, logging, collections
from config import topic, bootstrap_server, group_id


from kafka import KafkaConsumer, TopicPartition

logging.basicConfig(level=logging.ERROR)


def consumer_thread():
    consumer = KafkaConsumer(
        topic,
        bootstrap_servers=[bootstrap_server],
        auto_offset_reset='earliest',
        enable_auto_commit=False,
        group_id=group_id,
        value_deserializer=lambda m: json.loads(m.decode('utf-8')) if m else None,
        consumer_timeout_ms=20000
        )
    
    print("Python Kafka Consumer started. Waiting for messages...")
    

    try:
        for msg in consumer:
            payload = msg.value
            if not isinstance(payload, dict):
                print("Skip", payload)
                continue
        bids = payload.get("bids")
        asks = payload.get("asks")
    except:
        print("ERROR/NO-BID/ASKS")
    
        
   
    return bids, asks        
            
         
        






#jsut reading json data 
##for message in consumer:
##    print(f"Received message: Topic = {message.topic}, Partition={message.partition}, Offset={message.offset}")
 ##   print("Payload:", json.dumps(message.value, indent=2)) 
##    print("-" * 50)
   
   
    
 
       
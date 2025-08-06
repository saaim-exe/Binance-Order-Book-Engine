import json
import time 
import logging

from kafka import KafkaConsumer, TopicPartition

logging.basicConfig(level=logging.ERROR)

consumer = KafkaConsumer(
    'orderbook',
    bootstrap_servers=['localhost:9092'],
    auto_offset_reset='earliest',
    enable_auto_commit=False,
    group_id='temp-group-for-debug',
    value_deserializer=lambda m: json.loads(m.decode('utf-8')) if m else None,
    consumer_timeout_ms=20000
)

print("Python Kafka Consumer started. Waiting for messages...")

for message in consumer:
    print(f"Received message: Topic = {message.topic}, Partition={message.partition}, Offset={message.offset}")
    print("Payload:", json.dumps(message.value, indent=2)) 
    print("-" * 50)






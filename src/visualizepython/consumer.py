from typing import Dict, Any
import json, threading, queue, logging
from kafka import KafkaConsumer
from config import topic, bootstrap_server, group_id
            
#jsut reading json data 
##for message in consumer:
##    print(f"Received message: Topic = {message.topic}, Partition={message.partition}, Offset={message.offset}")
 ##   print("Payload:", json.dumps(message.value, indent=2)) 
##    print("-" * 50)
   
logging.basicConfig(level=logging.ERROR)

def consumer_thread(out_q: "queue.Queue[Dict[str, Any]]", stop_evt: threading.Event):
    consumer = KafkaConsumer(
        topic,
        bootstrap_servers=[bootstrap_server],
        auto_offset_reset='earliest',
        enable_auto_commit=False,
        group_id=group_id,
        value_deserializer=lambda m: json.loads(m.decode('utf-8')) if m else None,
        consumer_timeout_ms=1000
        )
    
    print("Python Kafka Consumer started. Waiting for messages...")
    
    try:
        while not stop_evt.is_set():
            try:
                for message in consumer: 
                    print("Kafka -> message")
                    if stop_evt.is_set():
                        break 
                    payload = message.value
                    if not isinstance(payload, dict):
                        continue
                    bids = payload.get("bids") or []
                    asks = payload.get("asks") or [] 
                
                    best_bid = bids[0] if bids else None
                    best_ask = asks[0] if asks else None 

                    item: Dict[str, Any] = {
                        "bids": bids,
                        "asks": asks,
                        "spread": payload.get("spread"),
                        "mid": payload.get("mid"),
                        "bidDepth": payload.get("bidDepth") or [],
                        "askDepth": payload.get("askDepth") or [],
                        "best_bid_price": best_bid["price"] if best_bid else None,
                        "best_bid_quantity": best_bid["quantity"] if best_bid else None, 
                        "best_ask_price": best_ask["price"] if best_ask else None,
                        "best_ask_quantity": best_ask["quantity"] if best_ask else None
                    }
                
                    try:
                        print("enqueue:", len(bids), len(asks), flush=True)
                        out_q.put_nowait(item)  
                    except queue.Full:
                        try:
                            out_q.get_nowait()
                        except queue.Empty:
                            pass
                        out_q.put_nowait(item)
            except Exception as e: 
                print("Consumer loop error:", e)
    finally:
        try:
            consumer.close()
        except Exception: 
            pass
        print("Kafka Consumer stopped")


   
    
 
       
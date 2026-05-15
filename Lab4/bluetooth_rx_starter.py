import asyncio
from bleak import BleakClient
import csv
import time

DEVICE_ADDRESS = "2928A34F-98AE-5F41-1753-E3AA64B55766"
CHAR_UUID = "12345678-1234-1234-1234-1234567890ac"

csvfile = open("/Users/nimalan/Desktop/IOT_Lab/indoor.csv", "a", newline="")
writer = csv.writer(csvfile)
writer.writerow(["Time_s", "Voltage_V", "Current_mA", "Power_mW"])

start_time = None

def handle_notify(_, data):
    global start_time
    
    if start_time is None:
        start_time = time.time()
        elapsed_time = 0.000
    else:
        elapsed_time = time.time() - start_time

    decoded_data = data.decode("utf-8").split(",")
    row_to_write = [f"{elapsed_time:.2f}"] + decoded_data
    writer.writerow(row_to_write)

    print(f"Time: {elapsed_time:.2f}s | V, I, P: {decoded_data}")

async def main():
    client = BleakClient(DEVICE_ADDRESS)
    try:
        await client.connect()
        print("Connected:", client.is_connected)

        await client.start_notify(CHAR_UUID, handle_notify)
        print("Receiving data for 60s...")
        

        await asyncio.sleep(60) 

    finally:
        if client.is_connected:
            try:
                await client.stop_notify(CHAR_UUID)
            except Exception as e:
                print("stop_notify error:", e)

            await client.disconnect()
            print("Disconnected cleanly")
        
        csvfile.close()
        print("Data saved to", csvfile.name)

            
try:
    asyncio.run(main())
except KeyboardInterrupt:
    print("Stopped by user")
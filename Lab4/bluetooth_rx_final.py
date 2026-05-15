import asyncio
from bleak import BleakClient
import csv
import time

DEVICE_ADDRESS = "2928A34F-98AE-5F41-1753-E3AA64B55766"
CHAR_UUID = "12345678-1234-1234-1234-1234567890ac"

csvfile = open("/Users/nimalan/Desktop/IOT_Lab/power_control.csv", "a", newline="")
writer = csv.writer(csvfile)
writer.writerow(["Time_s", "Voltage_V", "Current_mA", "Power_mW", "Fan_State"])

start_time = None
avg_time = 10.0

def handle_notify(_, data):
    global start_time, avg_time
    
    if start_time is None:
        start_time = time.time()
        elapsed_time = 0.000
    else:
        elapsed_time = time.time() - start_time


    decoded_data = data.decode("utf-8").split(",")

    # Check if the packet is the 10s average packet
    if decoded_data[0] == "AVG":
        avgV = decoded_data[1]
        avgI = decoded_data[2]
        avgP = decoded_data[3]
        fanState = decoded_data[4]

        print(f"Time: {avg_time:.2f}s | 10s AVG V, I, P, Fan State: {avgV}, {avgI}, {avgP}, {fanState}")
        avg_time += 10.0 

    # If it is raw data packet then write to CSV
    elif len(decoded_data) == 4:
        row_to_write = [f"{elapsed_time:.2f}"] + decoded_data
        writer.writerow(row_to_write)

async def main():
    client = BleakClient(DEVICE_ADDRESS)
    try:
        await client.connect()
        print("Connected:", client.is_connected)

        await client.start_notify(CHAR_UUID, handle_notify)
        print("Receiving data for 10min...")
        

        await asyncio.sleep(600) 

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
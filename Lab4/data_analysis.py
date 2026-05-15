import pandas as pd
import matplotlib.pyplot as plt

def analyse_power(csv_filename):

    dataframe = pd.read_csv(csv_filename)
    
    avg_voltage = dataframe['Voltage_V'].mean()
    avg_current = dataframe['Current_mA'].mean()
    avg_power = dataframe['Power_mW'].mean()
    
    print(f"Overall Average Voltage: {avg_voltage:.3f} V")
    print(f"Overall Average Current: {avg_current:.2f} mA")
    print(f"Overall Average Power:   {avg_power:.2f} mW\n")
    
    # Create a new column for 10-second bins
    dataframe['Time_Bin_10s'] = (dataframe['Time_s'] // 10) * 10 

    # Calculate the average power for each 10-second bin
    avg_10s = dataframe.groupby('Time_Bin_10s')['Power_mW'].mean().reset_index()
    print("--- 10-Second Average Power ---")
    print(avg_10s)
    
    plt.figure(figsize=(10, 6))
    
    # Plot the raw 20ms data
    plt.plot(dataframe['Time_s'], dataframe['Power_mW'], linestyle='-', label='Raw Power data', color='red')

    plt.title(f"Raw Power Consumption")
    plt.xlabel("Time (seconds)")
    plt.ylabel("Power (mW)")
    plt.legend()
    plt.grid(True, linestyle=':', alpha=0.6)
    plt.tight_layout()
    
    plt.figure(figsize=(10, 6))
    
    # Plot the 6 averaged 10s points
    plt.plot(avg_10s['Time_Bin_10s'] + 10, avg_10s['Power_mW'], 
             color='red', linestyle='-', marker='o', markersize=8, 
             label='10s Average Power')
    
    plt.title(f"10-Second Average Power")
    plt.xlabel("Time (seconds)")
    plt.ylabel("Power (mW)")
    plt.legend()
    plt.grid(True, linestyle=':', alpha=0.6)
    plt.tight_layout()
    
    plt.show()


csv_path = "/Users/nimalan/Desktop/IOT_Lab/power_control.csv"
analyse_power(csv_path)
import pandas as pd
import matplotlib.pyplot as plt

def analyse_power(csv_filename):

    dataframe = pd.read_csv(csv_filename)

    # Convert fan state to numeric values for easier analysis
    def convert_fan_state(state):
        state_str = str(state)
        if state_str == "OFF": return 0
        if state_str == "25%": return 25
        if state_str == "50%": return 50
        if state_str == "100%": return 100
        return 0

    if 'Fan_State' in dataframe.columns:
        dataframe['Fan_State_Num'] = dataframe['Fan_State'].apply(convert_fan_state)
    
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

    # Plot power consumption with both raw and 10s average data
    plt.figure(figsize=(10, 5))
    plt.plot(dataframe['Time_s'], dataframe['Power_mW'], linestyle='-', label='Raw Power data', color='lightcoral')
    plt.plot(avg_10s['Time_Bin_10s'], avg_10s['Power_mW'], color='darkred', linestyle='-', label='10s Average Power')
    plt.title("Power Consumption")
    plt.xlabel("Time (seconds)")
    plt.ylabel("Power (mW)")
    plt.legend()
    plt.grid(True, linestyle=':', alpha=0.6)
    plt.tight_layout()

    # Plot voltage
    plt.figure(figsize=(10, 5))
    plt.plot(dataframe['Time_s'], dataframe['Voltage_V'], linestyle='-', color='blue')
    plt.title("Battery Voltage")
    plt.xlabel("Time (seconds)")
    plt.ylabel("Voltage (V)")
    plt.grid(True, linestyle=':', alpha=0.6)
    plt.tight_layout()

    # Plot current
    plt.figure(figsize=(10, 5))
    plt.plot(dataframe['Time_s'], dataframe['Current_mA'], linestyle='-', color='orange')
    plt.title("System Current")
    plt.xlabel("Time (seconds)")
    plt.ylabel("Current (mA)")
    plt.grid(True, linestyle=':', alpha=0.6)
    plt.tight_layout()

    # Plot fan state
    if 'Fan_State_Num' in dataframe.columns:
        plt.figure(figsize=(10, 5))
        plt.step(dataframe['Time_s'], dataframe['Fan_State_Num'], where='post', color='green', linewidth=2)
        plt.title("Fan Duty Cycle")
        plt.xlabel("Time (seconds)")
        plt.ylabel("Speed (%)")
        plt.yticks([0, 25, 50, 100], ['OFF', '25%', '50%', '100%'])
        plt.grid(True, linestyle=':', alpha=0.6)
        plt.tight_layout()

    plt.show()

csv_path = "/Users/nimalan/Desktop/IOT_Lab/power_control.csv"
analyse_power(csv_path)
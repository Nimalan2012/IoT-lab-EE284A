#include <QuickMedianLib.h>

// Define the ADC pin
const int ADC_PIN = A2;

// Set the reference voltage
const float V_REF = 2.1;

// Set total samples
const int TOTAL_SAMPLES = 5000;

// Filter Numbers
const int N_50 = 50;
const int N_500 = 500;

// Buffers for Simple Moving Averages (SMA)
float sma50_buffer[N_50];
float sma500_buffer[N_500];
int sma50_index = 0, sma500_index = 0;
float sma50_sum = 0, sma500_sum = 0;

// Buffers for Moving Medians
float med50_buffer[N_50];
float med500_buffer[N_500];
int med50_index = 0, med500_index = 0;

// Variables for EMA
float ema50_val = 0;
float ema500_val = 0;
const float alpha_50 = 2.0 / (N_50 + 1.0);
const float alpha_500 = 2.0 / (N_500 + 1.0);

// Automatically calculates max, min, mean, and std dev 
struct FilterStats {
  int count = 0;
  float min_val = 1000.0;
  float max_val = -1000.0;
  double mean = 0.0;
  double M2 = 0.0; // Used for standard deviation

  void update(float val) {
    if (count == 0) {
      min_val = val;
      max_val = val;
    } else {
      if (val < min_val) min_val = val;
      if (val > max_val) max_val = val;
    }
    count++;
    double delta = val - mean;
    mean += delta / count;
    double delta2 = val - mean;
    M2 += delta * delta2;
  }

  float getStdDev() {
    if (count < 2) return 0.0;
    return sqrt(M2 / (count - 1));
  }
};

// Create a tracker for each of the 7 datasets
FilterStats rawStats, sma50Stats, sma500Stats, med50Stats, med500Stats, ema50Stats, ema500Stats;

// Function to print rows cleanly
void printTableRow(String name, FilterStats stats) {
  if (name.length() < 10) name += "\t"; 
  Serial.print(name + "\t");
  Serial.print(stats.max_val, 2); Serial.print("\t");
  Serial.print(stats.min_val, 2); Serial.print("\t");
  Serial.print(stats.mean, 2); Serial.print("\t");
  Serial.println(stats.getStdDev(), 4);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  analogReadResolution(12);

  //analogSetPinAttenuation(ADC_PIN, ADC_6db); 
  delay(50);

  // Convert ADC values input into voltage
  float voltage = analogReadMilliVolts(ADC_PIN);
  // Convert voltage into temperature based on TMP36 output charactersistics. The offset of 0.5V is subtracted off.
  float initalTemperature = (voltage - 500.0)/ 10.0;

  for (int i = 0; i < N_500; i++) {
    if (i < N_50) {
      sma50_buffer[i] = initalTemperature;
      sma50_sum += initalTemperature;
      med50_buffer[i] = initalTemperature;
    }
    sma500_buffer[i] = initalTemperature;
    sma500_sum += initalTemperature;
    med500_buffer[i] = initalTemperature;
  }
  ema50_val = initalTemperature;
  ema500_val = initalTemperature;

  Serial.println("Starting 5000 sample collection...");
}


void loop() {
  // put your main code here, to run repeatedly:

  // 5000 samples should only run once and not infinitely
  static bool experimentComplete = false;

  if (!experimentComplete) {
    for (int i = 0; i < TOTAL_SAMPLES; i++) {
      
      // Read Raw Sensor Data
      //analogSetPinAttenuation(ADC_PIN, ADC_6db);
      int currentVoltage = analogReadMilliVolts(ADC_PIN);
      float currentTemp = (currentVoltage - 500.0)/ 10.0;
      rawStats.update(currentTemp);

      // Simple Moving Average (N=50)
      sma50_sum -= sma50_buffer[sma50_index];
      sma50_buffer[sma50_index] = currentTemp;
      sma50_sum += currentTemp;
      sma50_index = (sma50_index + 1) % N_50;
      sma50Stats.update(sma50_sum / N_50);

      // Simple Moving Average (N=500)
      sma500_sum -= sma500_buffer[sma500_index];
      sma500_buffer[sma500_index] = currentTemp;
      sma500_sum += currentTemp;
      sma500_index = (sma500_index + 1) % N_500;
      sma500Stats.update(sma500_sum / N_500);

      // Moving Median (N=50)
      med50_buffer[med50_index] = currentTemp;
      med50_index = (med50_index + 1) % N_50;
      float tempMed50[N_50];
      memcpy(tempMed50, med50_buffer, sizeof(med50_buffer)); 
      float median50 = QuickMedian<float>::GetMedian(tempMed50, N_50);
      med50Stats.update(median50);

      // Moving Median (N=500)
      med500_buffer[med500_index] = currentTemp;
      med500_index = (med500_index + 1) % N_500;
      float tempMed500[N_500];
      memcpy(tempMed500, med500_buffer, sizeof(med500_buffer));
      float median500 = QuickMedian<float>::GetMedian(tempMed500, N_500);
      med500Stats.update(median500);

      // Exponential Moving Average (N=50)
      ema50_val = (alpha_50 * currentTemp) + ((1.0 - alpha_50) * ema50_val);
      ema50Stats.update(ema50_val);

      // Exponential Moving Average (N=500)
      ema500_val = (alpha_500 * currentTemp) + ((1.0 - alpha_500) * ema500_val);
      ema500Stats.update(ema500_val);

      delay(1);
    }

    // Results table
    Serial.println(String("Method") + "\t\t" + "Max(C)" + "\t" + "Min(C)" + "\t" + "Mean(C)" + "\t" + "StdDev");

    printTableRow("Raw Data  ", rawStats);
    printTableRow("SMA (N=50)", sma50Stats);
    printTableRow("SMA (N=500)", sma500Stats);
    printTableRow("Med (N=50)", med50Stats);
    printTableRow("Med (N=500)", med500Stats); 
    printTableRow("EMA (N=50)", ema50Stats);
    printTableRow("EMA (N=500)", ema500Stats);
    
    // Makes the experiment stop and doesn't infinitely loop
    experimentComplete = true; 
  }
}

import serial 
import time
import pandas as pd
import plotly.express as px

# Open Serial Port 
ser = serial.Serial(port="/dev/tty.usbserial-0001", baudrate=9600, bytesize=8, timeout=2, stopbits=serial.STOPBITS_ONE)

# Initializations
dataStr = ''    # String to store received uint16_t numbers 
adcList = []    # List to store received adc values
perList = []    # List to store received light intesnity percentages
timeList = []   # List to store received time stamps of received uint16_t numbers

line_count = 0   # Counter to track the number of lines processed

startTime = time.time()  # Start tracking time

# Capture UART Data
while time.time() - startTime < 60:  # Record data for 60 seconds
    line = ser.readline()  # Reads uint16_t nums as single bytes until \n and stores in string
    
    # Increment the line count after reading a line
    line_count += 1
    
    # Skip processing the first two lines in case they are not read properly
    if line_count < 3:
        continue

    if ((line != b' \n') and (line != b'\n')):  # Removes any '\n' without num captures
        dataStr += line.decode('Ascii')  # Converts string of received uint16_t num to ASCII and combines Rx nums into 1 string
        timeMeas = time.time() - startTime  # Time stamp received number
        timeList.append(timeMeas)  # Save time stamps in a list

# Close Serial Port    
ser.close() 

# UART Data Cleanup 
dataStr = dataStr.strip()  # Remove unwanted chars and spaces 
dataList = dataStr.split('\n')  # Split string by \n and store in list

print(dataList)
print(timeList)
print(len(dataList))
print(len(timeList))
print()

# Seperate percentage and adc values
perList = [entry.split('A')[0].strip().replace('P', '') for entry in dataList]
adcList = [entry.split('A')[1].strip() if 'A' in entry else '' for entry in dataList]

# This for loop also works to seperate
# for item in dataList:
#     perList.append(item[2:6])
#     adcList.append(item[10:14])

print(perList)
print(adcList)
print(len(perList))
print(len(adcList))

# UART Data float conversion
adcList = [float(elem) for elem in adcList]
perList = [float(elem) for elem in perList]

# Create dataframe using ADC and time data lists
adcValue = pd.DataFrame()
adcValue['Time (sec)'] = timeList
adcValue["ADC Value (decimal)"] = adcList

# Create dataframe using light intesity percentage and time data lists
lightPercent = pd.DataFrame()
lightPercent['Time (sec)'] = timeList
lightPercent["Light Intensity (%)"] = perList

# Combine data into a single DataFrame
dataFrame = pd.DataFrame({
    'Time (sec)': timeList,
    'ADC Value (decimal)': adcList,
    'Light Intensity (%)': perList
})

# Save to a single CSV
dataFrame.to_csv("ADC_and_Intensity_Data.csv", index=False)

# Plot and show ADC value vs Time using dataframe
fig = px.line(adcValue, x='Time (sec)', y="ADC Value (decimal)", title="ADC Value vs Time")
fig.show()

# Plot and show light intesity percentage vs Time using dataframe
fig = px.line(lightPercent, x='Time (sec)', y='Light Intensity (%)', title='Light Intesity Percentage vs Time')
fig.show()


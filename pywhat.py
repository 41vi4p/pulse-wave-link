import firebase_admin
from firebase_admin import credentials, db
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import pandas as pd
from datetime import datetime
import json

# Initialize Firebase Admin SDK with JSON file
cred = credentials.Certificate("pulse-wave-link-firebase-adminsdk-jyoc7-60aae8b349.json")
firebase_admin.initialize_app(cred, {'databaseURL': 'https://pulse-wave-link-default-rtdb.firebaseio.com/'})

# Initialize Firebase references
bpm_ref = db.reference('health/bpm')
spo_ref = db.reference('health/SpO2')
tem_ref = db.reference('health/tem')

# Data storage
data = {
    'time': [],
    'bpm': [],
    'SpO2': [],
    'temperature': []
}

# Callback functions to handle data changes
def bpm_listener(event):
    data['time'].append(datetime.now().isoformat())
    data['bpm'].append(event.data)
    data['SpO2'].append(None)
    data['temperature'].append(None)
    save_data_to_json()

def spo_listener(event):
    data['time'].append(datetime.now().isoformat())
    data['bpm'].append(None)
    data['SpO2'].append(event.data)
    data['temperature'].append(None)
    save_data_to_json()

def tem_listener(event):
    data['time'].append(datetime.now().isoformat())
    data['bpm'].append(None)
    data['SpO2'].append(None)
    data['temperature'].append(event.data)
    save_data_to_json()

# Function to save data to JSON file
def save_data_to_json():
    with open('sensor_data.json', 'w') as json_file:
        json.dump(data, json_file, indent=4)

# Set up listeners
bpm_ref.listen(bpm_listener)
spo_ref.listen(spo_listener)
tem_ref.listen(tem_listener)

# Plotting function
def animate(i):
    plt.cla()
    df = pd.DataFrame(data)
    df['time'] = pd.to_datetime(df['time'])
    df.set_index('time', inplace=True)
    df.interpolate(method='time', inplace=True)
    df.plot(ax=plt.gca())
    plt.xlabel('Time')
    plt.ylabel('Sensor Values')
    plt.title('Real-time Sensor Data')
    plt.legend(['BPM', 'SpO2', 'Temperature'])

# Apply dark theme
plt.style.use('dark_background')

# Set up the plot
fig = plt.figure()
ani = animation.FuncAnimation(fig, animate, interval=1000)

# Show the plot
plt.show()
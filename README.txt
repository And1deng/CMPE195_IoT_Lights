IoT Lighting System

Abstract:

Our project is an IoT lighting system that utilizes two ESP32s for sensor readings, an Arduino UNO R4 as a main controller hub, and a Raspberry Pi 3 B+ for a cloud database. The ESP32s are connected to a single sensor each (one being a TSL2591 light sensor, the other being an HC-SR501 PIR motion sensor) to communicate any sensor readings/detections from a portable battery (or wired) module to the main Arduino hub. The Arduino processes these sensor readings and, depending on which mode the system is set in (via the user), behaves accordingly. Data from these sensors is also passed to the cloud database for data collection and storage for future analytics. The system is enclosed in a 3D-printed housing for each component for ease of use and component protection.

Hardware components
Arduino Uno R4 WiFi
ESP32-WROOM32 (x2)
TSL2591 Light sensor
HC-SR501 PIR motion sensor
Raspberry Pi 3 B+
Wires
Breakout Boards


ESP32s
	The ESP32s are used to read sensor data (Lux values for light, boolean for motion) and relay that data through Arduino IoT cloud shared values to the main Arduino hub. They operate in a WiFi sleep mode to lower power consumption while maintaining cloud and internet connections for any variable updates and periodically update sensor data for the main hub.

Arduino
	The Arduino is the main processing unit of the system, handling several tasks such as: data storage, managing sensor modules, and determining system behavior. 
Sensor module management
 The Arduino has capabilities to completely enable or disable sensor modules as the user decides. This is to allow the user full functionality in cases where some sensors are not needed to increase energy saving for the system since they will not need to update any sensor readings.
System Behavior
The system operates in three main modes: manual, automatic, and scheduler. The manual mode allows the user to maintain full functionality by turning the light on/off and setting the brightness. Automatic mode is used to automatically adjust lighting to the user's preference through user-defined brightness thresholds. When the room is too dark, the algorithm will set the light to an estimated level to suffice for the difference in lux reading, then adjust accordingly if the threshold is not met (brightening the light), or is exceeded by over 30% of the set thresholds (dimming the light). Finally, scheduler mode is used to define a repeated, user-defined schedule (daily, weekly, etc) to enable the automatic mode only during those defined times. This is utilized to account for sleeping times as the system should not be trying to detect when to turn on lights during sleeping times, as well as when it is not needed during bright daytime.

Cloud Database
	The cloud implementation involves setting up a communication pipeline between ESP32 sensors, an Arduino Uno R4, a Raspberry Pi server, and a MongoDB database. The ESP32 sensors first gather light and motion readings from the surrounding environment, and then the Arduino collects these readings and sends them to the Raspberry Pi server via HTTP POST requests. The Raspberry Pi then validates this data and forwards it to the MongoDB database for storage.

	To set up this backend server, you first need to create a MongoDB account and create a cluster and database. After this, you need to install Node.js on the system where you want to run the server. After installing this, in the command terminal, run the command “npm install express mongoose cors dotenv” to install the necessary dependencies. After this, create a folder to contain the server code, and add the server.js and .env files to it. In the .env file, assign the MONGODB_URL variable to the MongoDB Atlas URL that is associated with your MongoDB cluster. After this, all that needs to be done is to go into the command terminal, move to the directory containing the two files, and then run the command “node server.js”.

User Interface
	The user interface provides customization to the system. This includes functions to toggle automatic vs manual mode, scheduling, setting thresholds based on the time of day, and overall management of the system. This allows the user to personalize their system to their needs and gives full access to the wide range of features our system provides. There is also a live dashboard that gives insights into data collected and makes it easy to view how the system is performing currently and how it has performed in the past.



Github Repository Link: https://github.com/And1deng/CMPE195_IoT_Lights.git

Youtube Demo Link: https://youtu.be/RhLekK8LELY 
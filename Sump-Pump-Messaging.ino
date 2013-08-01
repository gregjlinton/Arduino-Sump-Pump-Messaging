/*

 Arduino --> ThingSpeak Channel via Ethernet
 
 The ThingSpeak Client sketch is designed for the Arduino and Ethernet.
 This sketch updates a channel feed with an analog input reading via the
 ThingSpeak API (http://community.thingspeak.com/documentation/)
 using HTTP POST. The Arduino uses DHCP and DNS for a simpler network setup.
 The sketch also includes a Watchdog / Reset function to make sure the
 Arduino stays connected and/or regains connectivity after a network outage.
 Use the Serial Monitor on the Arduino IDE to see verbose network feedback
 and ThingSpeak connectivity status.
 
 Getting Started with ThingSpeak:
 
   * Sign Up for New User Account - https://www.thingspeak.com/users/new
   * Register your Arduino by selecting Devices, Add New Device
   * Once the Arduino is registered, click Generate Unique MAC Address
   * Enter the new MAC Address in this sketch under "Local Network Settings"
   * Create a new Channel by selecting Channels and then Create New Channel
   * Enter the Write API Key in this sketch under "ThingSpeak Settings"
 
 Arduino Requirements:
 
   * Arduino with Ethernet Shield or Arduino Ethernet
   * Arduino 1.0 IDE
   
  Network Requirements:

   * Ethernet port on Router    
   * DHCP enabled on Router
   * Unique MAC Address for Arduino
 
 Created: October 17, 2011 by Hans Scharler (http://www.iamshadowlord.com)
 
 Additional Credits:
 Example sketches from Arduino team, Ethernet by Adrian McEwen
 
*/

#include <SPI.h>
#include <Ethernet.h>

int sensorPin = A0;    // select the input pin current sensor
int LedPin = 13;       // selects the pin for the warning light LED
int threshold = 400;   // this sets the threshold value for the current sensor
int sensorValue;       // this is the variable for the current sensor right now
int last_sensorValue;  // this is the value that the current sensor was in the past
int timeout=6000;      // this is the time (in miliseconds) that we wait for the pump reservoir to drain. This should be well and truely past the normal time it takes to drain, like 1800000 (30 min)


// Local Network Settings
byte mac[] = { 0xD4, 0x28, 0xB2, 0xFF, 0xE0, 0x37 }; // Must be unique on local network

// ThingSpeak Settings
char thingSpeakAddress[] = "api.thingspeak.com";
String writeAPIKey = "xxxxxxxxxxxxxxxxxxx"; // replace this with the proper API Write Key from your own ThingSpeak Channel
const int updateThingSpeakInterval = 16 * 1000;      // Time interval in milliseconds to update ThingSpeak (number of seconds * 1000 = interval)

// Variable Setup
long lastConnectionTime = 0; 
boolean lastConnected = false;
int failedCounter = 0;
String badState="All Clear ";
String statusMessage=badState;
int randNumber=0;

// Initialize Arduino Ethernet Client
EthernetClient client;

void setup()
{
  // Start Serial for debugging on the Serial Monitor
  Serial.begin(9600);
  sensorValue = analogRead(sensorPin);        // now we're setting the current sensor variable to the value read in from the input pin
  last_sensorValue = analogRead(sensorPin);   // we have to start with both the present and past tense values of the variable the same at the start of the program
  randomSeed(analogRead(3)); // this generates a random seed value for number generation. We need that for unique data for tweets, otherwise they get scrubbed.
  
  // Start Ethernet on Arduino
  startEthernet();
}

void loop()
{
  // Read value from Analog Input Pin 0
  String SensorReading = String(analogRead(A0), DEC);
  int sensorValue;
  
  sensorValue = analogRead(sensorPin);        //as part of the loop we're setting the present sensor reading
  
  if((sensorValue>threshold)&&(last_sensorValue<=threshold)){   // this starts a while loop so that if the sensor reading goes up past the threshold we do something
   delay (timeout);                                               // this delays our next if loop based on how it takes the reservoir to drain.
      if (sensorValue>threshold){                                 // now we double check to see if its still above our threshold value
           Serial.println("Pump Danger");                         // the rest of this is the bad things that happen. Effectivly this is the "Oh Crap!" message
           badState="Pump Danger ";
           randNumber = random(3000);                             // this is a random number generator
           statusMessage = badState + randNumber;                 // that gets tacked onto the status message, so that each one look unique, otherwise Twitter won't post it.
           digitalWrite(LedPin,HIGH);
           Serial.print("Sensor Reading ");
           Serial.println(sensorValue);
           last_sensorValue = sensorValue;}                        // now we reset the sensor reading variables so we can actually exit the while loop if we want. This will need to be fixed if we want to have a second notification of "Oh Crap!"
      
}
     if ((sensorValue<threshold)&&(last_sensorValue>=threshold)){     // This while loop is similar to the first but its looking for a drop in sensor readings. Thus if the pump fixes itself, it sends the "All Clear"
      if (sensorValue<threshold){   
           Serial.println("All Clear");
           badState="All Clear ";
           randNumber = random(3000);
           statusMessage = badState + randNumber;
           digitalWrite(LedPin,LOW);
           Serial.print("Sensor Reading ");
           Serial.println(sensorValue);
           last_sensorValue = sensorValue; }
}
  
  // Print Update Response to Serial Monitor
  if (client.available())
  {
    char c = client.read();
    Serial.print(c);
  }

  // Disconnect from ThingSpeak
  if (!client.connected() && lastConnected)
  {
    Serial.println("...disconnected");
    Serial.println();
    
    client.stop();
  }
  
  
  
  
  // Update ThingSpeak
  if(!client.connected() && (millis() - lastConnectionTime > updateThingSpeakInterval))
  {
    updateThingSpeak("field1="+SensorReading+"&status="+statusMessage);
  }
  
  // Check if Arduino Ethernet needs to be restarted
  if (failedCounter > 3 ) {startEthernet();}
  
  lastConnected = client.connected();
}

void updateThingSpeak(String tsData)
{
  if (client.connect(thingSpeakAddress, 80))
  {         
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: "+writeAPIKey+"\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(tsData.length());
    client.print("\n\n");

    client.print(tsData);
    
    lastConnectionTime = millis();
    
    if (client.connected())
    {
      Serial.println("Connecting to ThingSpeak...");
      Serial.println();
      
      failedCounter = 0;
    }
    else
    {
      failedCounter++;
  
      Serial.println("Connection to ThingSpeak failed ("+String(failedCounter, DEC)+")");   
      Serial.println();
    }
    
  }
  else
  {
    failedCounter++;
    
    Serial.println("Connection to ThingSpeak Failed ("+String(failedCounter, DEC)+")");   
    Serial.println();
    
    lastConnectionTime = millis(); 
  }
}

void startEthernet()
{
  
  client.stop();

  Serial.println("Connecting Arduino to network...");
  Serial.println();  

  delay(1000);
  
  // Connect to network amd obtain an IP address using DHCP
  if (Ethernet.begin(mac) == 0)
  {
    Serial.println("DHCP Failed, reset Arduino to try again");
    Serial.println();
  }
  else
  {
    Serial.println("Arduino connected to network using DHCP");
    Serial.println();
  }
  
  delay(1000);
}

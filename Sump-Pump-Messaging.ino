// The basic idea here is to have a sump pump that tells you when something has gone wrong.
// Measuring depth and so on is interesting but not very practical.
// What you're intersted in is if the pump is failing.
// For that you need to know if the pump is going to burn itself out from running too long.
// To do that you need to know if its on. You won't get that from measuring depth.
// For example. If the exit line freezes the reservoir won't drain and the pump keeps running.
// Or the float can jam and the pump keeps trying to haul water from a dry reservoir.
// At this point the code only sends an alert if the pump has been running for too long, or if the pump has somehow cleared itself.
// But we'll have to add depth measurement later since if the pump has failed and the water level is still rising your basement will still flood.
// Next up is how to get the data off the Arduino and into the rest of the world.


int sensorPin = A0;    // select the input pin current sensor
int LedPin = 13;       // selects the pin for the warning light LED
int threshold = 400;   // this sets the threshold value for the current sensor
int sensorValue;       // this is the variable for the current sensor right now
int last_sensorValue;  // this is the value that the current sensor was in the past
int timeout=6000;      // this is the time (in miliseconds) that we wait for the pump reservoir to drain. This should be well and truely past the normal time it takes to drain, like 1800000 (30 min)

void setup() {
   Serial.begin(9600);                         // here we're setting up the serial port so we can read any printed output
   sensorValue = analogRead(sensorPin);        // now we're setting the current sensor variable to the value read in from the input pin
   last_sensorValue = analogRead(sensorPin);   // we have to start with both the present and past tense values of the variable the same at the start of the program
}

void loop() {
  int sensorValue;
  sensorValue = analogRead(sensorPin);        //as part of the loop we're setting the present sensor reading
  
 
 while((sensorValue>threshold)&&(last_sensorValue<=threshold)){   // this starts a while loop so that if the sensor reading goes up past the threshold we do something
   delay (timeout);                                               // this delays our next if loop based on how it takes the reservoir to drain.
      if (sensorValue>threshold){                                 // now we double check to see if its still above our threshold value
           Serial.println("Pump Danger");                         // the rest of this is the bad things that happen. Effectivly this is the "Oh Crap!" message
           digitalWrite(LedPin,HIGH);
           Serial.print("Sensor Reading ");
           Serial.println(sensorValue);
           last_sensorValue = sensorValue;}                        // now we reset the sensor reading variables so we can actually exit the while loop if we want. This will need to be fixed if we want to have a second notification of "Oh Crap!"
      
}

  while ((sensorValue<threshold)&&(last_sensorValue>=threshold)){     // This while loop is similar to the first but its looking for a drop in sensor readings. Thus if the pump fixes itself, it sends the "All Clear"
      if (sensorValue<threshold){   
           Serial.println("All Clear");
           digitalWrite(LedPin,LOW);
           Serial.print("Sensor Reading ");
           Serial.println(sensorValue);
           last_sensorValue = sensorValue; }
}
}

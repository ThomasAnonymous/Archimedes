/* 
    Archimedes v1.0
        Automatic plant waterer
    fun things to do:
    - Add a pump that tunrs on after any valve is open and off before any valves close

 */

#include <Arduino.h>        // Arduino core v1.00+

// Pre-compile arguments. Stick to development metadata, not program values
#define DEBUG       1               // Debug flag 
#define ON          1               // Alias
#define OFF         0               // Alias

// Constants
const float DRY_VALUE   =605;//600,605           // Calibrate in air
const float WET_VALUE   =295;//291,295           // Calibrate in water
const float HUM_RANGE   =(DRY_VALUE - WET_VALUE);// Range of possible values

const int loopDelay     =1;                     // Loop delay in seconds
const float alpha       =0.0166;                // Rolling average coefficient (portion that each new value counts toward average)
const int relayPins[]   = {2,3};//,4,5,6,7};//,3,4,5};        // Relay output pins
const int sensorPins[]  = {A2,A3};//,A4,A5,A6,A7};//,A1,A2,A3};    // Sensor input pins

//Globals
bool    relayStates[]   ={OFF,OFF,OFF,OFF};     // Relay states
float   sensorLevels[]  ={0,0,0,0};             // Sensor input levels
int     targetLevels[]  ={60,60,50,50};         // Maximum moisture percentages
int     triggerLevels[] ={40,40,20,20};         // Minimum allowed levels
float   accumLevels[]   ={50,50,40,40};         // Accumulated level moving average


void setup(){
    for (int channel = 0; channel < (sizeof(relayPins)/sizeof(*relayPins)); channel++) { // Setup board pins based on arrays above
        pinMode(relayPins[channel], OUTPUT);    // Set up output pins
        setRelay(channel,OFF);                  // Defualt relay to off
        pinMode(sensorPins[channel], INPUT);    // Set up output pins
    }

    #if DEBUG
        Serial.begin(9600);     // initiate Serial coms
        while (!Serial);        // Until serial output begins...
        delay(200);             // Just wait
    #endif
    
}

void loop(){
    checkLevels();              // Check sensors for moisture levels
    checkValves();              // Set valve states
    delay(loopDelay*1000);      // Slow down our loop 
}

/* Check the sensor levels from each input and update our global sensor level array */
void checkLevels(){
    for (int channel = 0; channel < (sizeof(sensorPins)/sizeof(*sensorPins)); channel++) { // For each channel
        int reading=analogRead(channel);                            // Read level
        float percent=100*(1-((reading - WET_VALUE)/HUM_RANGE));    // Convert to moisture percent
        sensorLevels[channel]=percent;                              // Store result in sensor level
        accumLevels[channel] = (alpha * sensorLevels[channel]) + (1.0 - alpha) * accumLevels[channel]; // Each new value replaces a portion of the rolling average
        #if DEBUG
            Serial.print(" Channel ");Serial.print(channel);
            Serial.print(" = ");Serial.print(percent);
        #endif
    }
    #if DEBUG
        Serial.println("");
    #endif
}

void checkValves(){
    for (int channel = 0; channel < (sizeof(relayPins)/sizeof(*relayPins)); channel++) { // For each channel
        if(accumLevels[channel]<=triggerLevels[channel]){          // If the water level is too low
            relayStates[channel]=ON;                                    // Set the relay flag
            setRelay(channel,ON);                                       // Turn on the valve
        } else if (accumLevels[channel]>=targetLevels[channel]) {  // If the water level is sufficient
            relayStates[channel]=OFF;                                   // Clear the relay flag
            setRelay(channel,OFF);                                      // Turn off the valve
        }
        #if DEBUG
            Serial.print(" Accumul ");Serial.print(channel);
            Serial.print(" = ");
            Serial.print(accumLevels[channel]);
            
        #endif
    }
    #if DEBUG
        Serial.println("");
    #endif
}

/* Set an output channel to a given state. INVERTS FOR RELAY*/
void setRelay(int channel,bool state){
    digitalWrite(relayPins[channel],!state);
}
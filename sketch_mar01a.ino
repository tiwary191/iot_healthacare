#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Ticker.h>
//#include <BlynkSimpleEsp8266.h>

#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`

// Initialize the OLED display using Wire library
SSD1306  display(0x3c, D4, D5);

Ticker flipper;
const int httpPort = 80;
#define myPeriodic 15
#define ONE_WIRE_BUS 5  // DS18B20 on arduino pin2 corresponds to D4 on physical board
//#define mySSR 0  // Solid State Relay on pin 0

//#define BLYNK_PRINT Serial

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
float prevTemp = 0;

const char* server = "api.thingspeak.com";

Ticker sender;
const int maxAvgSample = 20;
volatile int rate[maxAvgSample];                    // used to hold last ten IBI values
boolean sendok = false;
volatile unsigned long sampleCounter = 0;          // used to determine pulse timing
volatile unsigned long lastBeatTime = 0;           // used to find the inter beat interval
volatile int P =512;                      // used to find peak in pulse wave
volatile int T = 512;                     // used to find trough in pulse wave
volatile int thresh = 512;                // used to find instant moment of heart beat
volatile int amp = 100;                   // used to hold amplitude of pulse waveform
volatile boolean firstBeat = true;        // used to seed rate array so we startup with reasonable BPM
volatile boolean secondBeat = true;       // used to seed rate array so we startup with reasonable BPM
volatile int BPM;                   // used to hold the pulse rate
volatile int Signal;                // holds the incoming raw data
volatile int IBI = 600;             // holds the time between beats, the Inter-Beat Interval
volatile boolean Pulse = false;     // true when pulse wave is high, false when it's low
volatile boolean QS = false;  

float temp=0;
unsigned long previousMillis = 0;
long interval = 2000; // interval at which to do something (milliseconds)


String apiKey ="HJCMGYQ6AAY980OT";
const char* ssid = "iothealthcare";
const char* password = "savelife";
WiFiClient client;
const char* host = "184.106.153.149";//you pc IP
//char auth[] = "6bea005419574ca69b6c25ba244bb56a ";

void setup() {
  Serial.begin(115200);  
  delay(10);
  display.init();

  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);

 
  display.drawString(24,0,"IOT Healthcare");
  Serial.print("Connecting to ");
  display.drawString(24, 16,"Connecting..");
  Serial.println(ssid);
  WiFi.begin(ssid, password); 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  display.drawString(24, 36,"Wifi Connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
 
  
  Serial.print("connecting to ");
  Serial.println(host);

  
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
   //Blynk.begin(auth, ssid, password);
  
  //client.print("255");
  delay(10);
    
  Serial.println();
  Serial.println("end Setup");
  flipper.attach_ms(2, Test);
  sender.attach(2, senderfunc);
}


void displayData(float a,float b){
  
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(24, 0,"Medical Data");
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 13,"Pulse:"+String(a)+"BPM");
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 36,"Temp:"+String(b)+" C");
  

  
  
  }

void senderfunc()
{
  sendok=true;
}



void loop() {  
  unsigned long currentMillis = millis();

  

   if(sendok)
     {
        delay(100);
            WiFiClient client;
              const int httpPort = 80;// to port to send the data
              if (!client.connect("api.thingspeak.com", httpPort)) {
                Serial.println("connection failed");
                return;
              }
   float temp=0;
  //char buffer[10];
  DS18B20.requestTemperatures(); 
  temp = DS18B20.getTempCByIndex(0);
  
  
 
  //String tempC = dtostrf(temp, 4, 1, buffer);//handled in sendTemp()
  Serial.print(" Temperature: ");
  Serial.println(temp);
  Serial.print(" BPM: ");
  Serial.println(BPM);
 if(currentMillis - previousMillis > interval) {
     previousMillis = currentMillis;  
    display.clear();
    displayData(BPM,temp); // do something
  }
  
 

//  if(temp>35)
//  { 
//    client.connect(host, httpPort);
// String tsData="api_key=";
////tsData = "api_key="+"P4Y1LZXTON2QIJ5X"+"&status="+"Temp critical,messege send by esp8266";
////tsData += "api_key";
//tsData += "P4Y1LZXTON2QIJ5X";
//tsData += "&status=";
//tsData += "temp critical, message sent via esp8266 IOT";
//            
//    client.print("POST /apps/thingtweet/1/statuses/update HTTP/1.1\n");
//    client.print("Host: api.thingspeak.com\n");
//    client.print("Connection: close\n");
//    client.print("Content-Type: application/x-www-form-urlencoded\n");
//    client.print("Content-Length: ");
//    client.print(tsData.length());
//    client.print("\n\n");
//    client.print(tsData);
//     return;
//  }
 


String postStr1 = apiKey;
   postStr1 += "&field1=";
   postStr1 += String(temp);
   postStr1 += "&field2=";
   postStr1 += String(BPM);
   postStr1 += "\r\n\r\n";
   
   client.print("POST /update HTTP/1.1\n");
   client.print("Host: api.thingspeak.com\n");
   client.print("Connection: close\n");
   client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
   client.print("Content-Type: application/x-www-form-urlencoded\n");
   client.print("Content-Length: ");
   client.print(postStr1.length());
   client.print("\n\n");
   client.print(postStr1);
   delay(10);
     
   sendok =false;
   flipper.attach_ms(2, Test);
  
              
     }

 display.display();
 delay(10);
}


void sendtcp()
{
         
              
}


int count = 0;
void Test()
{
  count++;
  if(count ==1000)
  {
    flipper.detach();
    count =0;
    //sendtcp();
    sendok=true;
    
  }
  
      Signal = analogRead(A0);              // read the Pulse Sensor 
        sampleCounter += 2;                         // keep track of the time in mS with this variable
    int N = sampleCounter - lastBeatTime;       // monitor the time since the last beat to avoid noise

      if(Signal < thresh && N > (IBI/5)*3){       // avoid dichrotic noise by waiting 3/5 of last IBI
        if (Signal < T){                        // T is the trough
            T = Signal;                         // keep track of lowest point in pulse wave 
         }
       }
      
    if(Signal > thresh && Signal > P){          // thresh condition helps avoid noise
        P = Signal;                             // P is the peak
       }                                        // keep track of highest point in pulse wave
    
  //  NOW IT'S TIME TO LOOK FOR THE HEART BEAT
  // signal surges up in value every time there is a pulse
if (N > 250){                                   // avoid high frequency noise
  if ( (Signal > thresh) && (Pulse == false) && (N > (IBI/5)*3) ){        
    Pulse = true;                               // set the Pulse flag when we think there is a pulse
    //digitalWrite(blinkPin,HIGH);                // turn on pin 13 LED
    IBI = sampleCounter - lastBeatTime;         // measure time between beats in mS
    lastBeatTime = sampleCounter;               // keep track of time for next pulse
         
         if(firstBeat){                         // if it's the first time we found a beat, if firstBeat == TRUE
             firstBeat = false;                 // clear firstBeat flag
             return;                            // IBI value is unreliable so discard it
            }   
         if(secondBeat){                        // if this is the second beat, if secondBeat == TRUE
            secondBeat = false;                 // clear secondBeat flag
               for(int i=0; i<=maxAvgSample-1; i++){         // seed the running total to get a realisitic BPM at startup
                    rate[i] = IBI;                      
                    }
            }
          
    // keep a running total of the last 10 IBI values
    word runningTotal = 0;                   // clear the runningTotal variable    

    for(int i=0; i<=(maxAvgSample-2); i++){                // shift data in the rate array
          rate[i] = rate[i+1];              // and drop the oldest IBI value 
          runningTotal += rate[i];          // add up the 9 oldest IBI values
        }
        
    rate[maxAvgSample-1] = IBI;                          // add the latest IBI to the rate array
    runningTotal += rate[maxAvgSample-1];                // add the latest IBI to runningTotal
    runningTotal /= maxAvgSample;                     // average the last 10 IBI values 
    BPM = 60000/runningTotal;               // how many beats can fit into a minute? that's BPM!
    QS = true;                              // set Quantified Self flag 
    // QS FLAG IS NOT CLEARED INSIDE THIS ISR
    }                       
}

  if (Signal < thresh && Pulse == true){     // when the values are going down, the beat is over
      //digitalWrite(blinkPin,LOW);            // turn off pin 13 LED
      Pulse = false;                         // reset the Pulse flag so we can do it again
      amp = P - T;                           // get amplitude of the pulse wave
      thresh = amp/2 + T;                    // set thresh at 50% of the amplitude
      P = thresh;                            // reset these for next time
      T = thresh;
     }
  
  if (N > 2500){                             // if 2.5 seconds go by without a beat
      thresh = 512;                          // set thresh default
      P = 512;                               // set P default
      T = 512;                               // set T default
      lastBeatTime = sampleCounter;          // bring the lastBeatTime up to date        
      firstBeat = true;                      // set these to avoid noise
      secondBeat = true;                     // when we get the heartbeat back
     }
  
  //sei();                                     // enable interrupts when youre done!
}// end isr

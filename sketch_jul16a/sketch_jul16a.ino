#include <TinyGPSPlus.h>
#define RXD2 16
#define TXD2 17
HardwareSerial neogps(1);
char   datoCmd = 0;

#define NMEA 0

// The TinyGPSPlus object
TinyGPSPlus gps;

void setup() {
  Serial.begin(115200);
  neogps.begin(9600, SERIAL_8N1, RXD2, TXD2); 
  delay(3000);
}

void loop() {
  if (NMEA){
      while (neogps.available())
      {
      datoCmd  = (char)neogps.read(); 
      Serial.print (datoCmd );
      }
  }
  else{
    boolean newData = false;
    for (unsigned long start = millis(); millis() - start < 1000;)
    {
      while (neogps.available())
      {
        if (gps.encode(neogps.read()))
        {
          newData = true;         
        }
      }
    }  

    if(newData == true)
    {
      newData = false;
      Serial.println(gps.satellites.value());    
      Visualizacion_Serial();
    }
    else
    {
      
    }  
    
  }
}

void Visualizacion_Serial(void)
{ 
   
  if (gps.location.isValid() ==  1)
  {  
    Serial.print("Lat: ");
    Serial.println(gps.location.lat(),6);
    Serial.print("Lng: ");
    Serial.println(gps.location.lng(),6);  
    Serial.print("Speed: ");
    Serial.println(gps.speed.kmph());    
    Serial.print("SAT:");
    Serial.println(gps.satellites.value());
    Serial.print("ALT:");   
    Serial.println(gps.altitude.meters(), 0);     

    Serial.print("Date: ");
    Serial.print(gps.date.day()); Serial.print("/");
    Serial.print(gps.date.month()); Serial.print("/");
    Serial.println(gps.date.year());

    Serial.print("Hour: ");
    Serial.print(gps.time.hour()); Serial.print(":");
    Serial.print(gps.time.minute()); Serial.print(":");
    Serial.println(gps.time.second());
    Serial.println("---------------------------");
  }
  else
  {
    Serial.println("Sin señal gps");  
  }  

}
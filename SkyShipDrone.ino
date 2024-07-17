#include "BluetoothSerial.h"
#include <TinyGPS++.h>

#define RXD2 16
#define TXD2 17
#define NMEA 0 // The default baudrate of NEO-6M is 9600

HardwareSerial neogps(1);
TinyGPSPlus gps;  // the TinyGPS++ object
BluetoothSerial SerialBT;

char datoCmd = 0;
bool esperandoRespuesta = false;

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is disabled. To enable it, go to `Tools` > `Core Debug Level` > `Bluetooth` and select `Classic Bluetooth` or `Dual Mode`.
#endif

void btCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
  if (event == ESP_SPP_SRV_OPEN_EVT) {
    Serial.println("Cliente conectado.");
  } else if (event == ESP_SPP_DATA_IND_EVT) {
    Serial.print("Recibido: ");
    Serial.println((char *)param->data_ind.data);
    esperandoRespuesta = false; // Mark that a response has been received
  }
}

void setup() {
  Serial.begin(115200);
  neogps.begin(9600, SERIAL_8N1, RXD2, TXD2);
  
  // Initialize Bluetooth
  if (!SerialBT.begin("SkyShip Drone")) {
    Serial.println("¡Fallo en el inicio del Bluetooth!");
    while (1);
  }

  SerialBT.register_callback(btCallback);

  Serial.println("El dispositivo Bluetooth está listo para emparejarse.");
  Serial.println(F("ESP32 - GPS module"));
  delay(3000);
}

void loop() {
  if(SerialBT.connected() && !esperandoRespuesta){
    GPS();
  }else{ 
    if (SerialBT.connected()){
      Serial.println("Esperando respuesta del dispositivo");
      delay(1000);
    }else{
      Serial.println("Esperando a que se concete un dispositivo");
      delay(2000);
    }
    
  }
  
}

void GPS(){
  if (NMEA) {
    while (neogps.available()) {
      datoCmd = (char)neogps.read();
      Serial.print(datoCmd);
    }
  } else {
    boolean newData = false;
    for (unsigned long start = millis(); millis() - start < 1000;) {
      while (neogps.available()) {
        if (gps.encode(neogps.read())) {
          newData = true;
        }
      }
    }

    if (newData) {
      newData = false;
      //Serial.println(gps.satellites.value());
      visualizacionSerial();
      enviarDatosBT();
    }
  }
}
void visualizacionSerial() {
  if (gps.location.isValid()) {
    Serial.println("---------------------------");
    Serial.print("SAT:");
    Serial.println(gps.satellites.value());
    Serial.print("Lat: ");
    Serial.println(gps.location.lat(), 6);
    Serial.print("Lng: ");
    Serial.println(gps.location.lng(), 6);
    //Serial.print("ALT:");
    //Serial.println(gps.altitude.meters(), 0);
    Serial.print("Datetime: ");
    Serial.print(gps.date.day()); Serial.print("/");
    Serial.print(gps.date.month()); Serial.print("/");
    Serial.print(gps.date.year());Serial.print(" - ");
    Serial.print(gps.time.hour()); Serial.print(":");
    Serial.print(gps.time.minute()); Serial.print(":");
    Serial.println(gps.time.second());
    Serial.println("---------------------------");
  } else {
    Serial.println("Sin señal GPS");
  }
}

void enviarDatosBT() {
  // Send data to the connected Bluetooth device if not waiting for a response
  if (gps.location.isValid() && SerialBT.connected() && !esperandoRespuesta) {
    SerialBT.print("Latitud: ");
    SerialBT.println(gps.location.lat(), 6);
    SerialBT.print("Longitud: ");
    SerialBT.println(gps.location.lng(), 6);
    SerialBT.print("Datetime: ");
    SerialBT.print(gps.date.day()); SerialBT.print("/");
    SerialBT.print(gps.date.month()); SerialBT.print("/");
    SerialBT.print(gps.date.year());SerialBT.print(" - ");
    SerialBT.print(gps.time.hour()); SerialBT.print(":");
    SerialBT.print(gps.time.minute()); SerialBT.print(":");
    SerialBT.println(gps.time.second());
    
    Serial.println("Datos enviados a través de Bluetooth.");
    Serial.println("---------------------------");
    esperandoRespuesta = true;  // Mark that waiting for a response
  }
  delay(2000);  // Add a small delay to avoid very rapid continuous sending
}

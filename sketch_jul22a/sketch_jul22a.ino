#include <BluetoothSerial.h>
#include <ESP32Servo.h>

BluetoothSerial SerialBT;
Servo esc;

const int escPin = 18;
float altura = 0.0;
int tiempo = 0;
bool paramsReceived = false;

// Callback para manejar eventos Bluetooth
void btCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
  if (event == ESP_SPP_SRV_OPEN_EVT) {
    Serial.println("Cliente conectado.");
  } else if (event == ESP_SPP_DATA_IND_EVT) {
    Serial.print("Recibido: ");
    Serial.println((char *)param->data_ind.data);
    String input = String((char *)param->data_ind.data);
    input.trim();

    if (input.startsWith("HEIGHT:")) {
      altura = input.substring(7).toFloat();
      Serial.print("Altura recibida: ");
      Serial.println(altura);
    } else if (input.startsWith("AIRTIME:")) {
      tiempo = input.substring(8).toInt();
      Serial.print("Tiempo de vuelo recibido: ");
      Serial.println(tiempo);
      paramsReceived = true;
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Inicializar Bluetooth
  if (!SerialBT.begin("SkyShip Drone")) {
    Serial.println("¡Fallo en el inicio del Bluetooth!");
    while (1);
  }

  SerialBT.register_callback(btCallback);
  esc.attach(escPin, 1000, 2000); // El ESC se controla como un servo

  Serial.println("El dispositivo Bluetooth está listo para emparejarse.");
}

void loop() {
  if (paramsReceived) {
    controlMotor(altura, tiempo);
    paramsReceived = false; // Restablecer la bandera
  }
}

void controlMotor(float altura, int tiempo) {
  // Mapea la altura (0-5 metros) a un rango de velocidad del ESC (1000-2000)
  int velocidadObjetivo = map(altura * 100, 0, 500, 1000, 2000);
  int velocidadActual = 1000; // Comienza desde la velocidad mínima
  
  // Incrementa la velocidad suavemente
  while (velocidadActual < velocidadObjetivo) {
    esc.writeMicroseconds(velocidadActual);
    Serial.print("Acelerando: ");
    Serial.println(velocidadActual);
    velocidadActual += 10; // Ajusta el incremento para controlar la aceleración
    delay(50); // Ajusta el intervalo para controlar la velocidad de aceleración
  }

  // Mantiene el motor funcionando durante el tiempo especificado (máximo 20 segundos)
  delay(min(tiempo, 20) * 1000);
  
  // Desacelera el motor suavemente
  while (velocidadActual > 1000) {
    esc.writeMicroseconds(velocidadActual);
    Serial.print("Desacelerando: ");
    Serial.println(velocidadActual);
    velocidadActual -= 10; // Ajusta el decremento para controlar la desaceleración
    delay(50); // Ajusta el intervalo para controlar la velocidad de desaceleración
  }
  
  // Asegúrate de que el motor esté completamente detenido
  esc.writeMicroseconds(1000);
  Serial.println("Motor detenido.");
}

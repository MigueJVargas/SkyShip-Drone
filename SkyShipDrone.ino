#include "BluetoothSerial.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_bt_defs.h"
#include "esp_spp_api.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth está deshabilitado. Para habilitarlo, ve a `Tools` > `Core Debug Level` > `Bluetooth` y selecciona `Classic Bluetooth` o `Dual Mode`.
#endif

BluetoothSerial SerialBT;
bool esperandoRespuesta = false;

void btCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
  if (event == ESP_SPP_SRV_OPEN_EVT) {
    Serial.println("Cliente conectado.");
  } else if (event == ESP_SPP_DATA_IND_EVT) {
    Serial.print("Recibido: ");
    Serial.println((char *)param->data_ind.data);
    
    // Marcar que se ha recibido una respuesta
    esperandoRespuesta = false;
  }
}

void btGapCallback(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param) {
  switch (event) {
    case ESP_BT_GAP_PIN_REQ_EVT: {
      Serial.println("PIN requerido.");
      esp_bt_pin_code_t pin_code;
      strcpy((char *)pin_code, "1234");
      esp_bt_gap_pin_reply(param->pin_req.bda, true, 4, pin_code);
      break;
    }
    case ESP_BT_GAP_AUTH_CMPL_EVT: {
      if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS) {
        Serial.println("Autenticación exitosa.");
        Serial.print("Emparejado con: ");
        Serial.println((char *)param->auth_cmpl.device_name);
      } else {
        Serial.println("Autenticación fallida.");
      }
      break;
    }
    default: {
      break;
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  // Inicializar Bluetooth con el nombre "ESP32test"
  if (!SerialBT.begin("SkyShip Drone")) {
    Serial.println("¡Fallo en el inicio del Bluetooth!");
    while (1);
  }
  
  // Configurar el PIN y registrar los callbacks
  esp_bt_gap_register_callback(btGapCallback);
  esp_bt_gap_set_pin(ESP_BT_PIN_TYPE_FIXED, 4, (uint8_t *)"1234");
  SerialBT.register_callback(btCallback);

  Serial.println("El dispositivo Bluetooth está listo para emparejarse.");
}

void loop() {
  // Enviar datos al dispositivo Bluetooth conectado si no se está esperando una respuesta
  if (SerialBT.connected() && !esperandoRespuesta) {
    SerialBT.println("Hola desde SkyShip Drone!");
    delay(2000);  // Envía el mensaje "Hola desde SkyShip Drone!"
    esperandoRespuesta = true;  // Marcar que se está esperando una respuesta
  }
  
  // Manejar otros eventos o tareas si es necesario
  delay(2000); // Añadir un pequeño retardo para evitar envíos continuos muy rápidos
}

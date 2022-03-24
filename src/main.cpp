// CUBO MAGICO
//*********************

#include <Arduino.h>
#include <WiFi.h>
#include "driver/adc.h"         // Deep Sleep manual ESP32
#include <esp_wifi.h>           // Deep Sleep manual ESP32
#include <esp_bt.h>             // Deep Sleep manual ESP32
#include <AsyncMqttClient.h>    // MQTT Libreria asincrona basada en eventos
#include <ArduinoOTA.h>         // Actualización por OTA
#include <ESPmDNS.h>            // Actualización por OTA
#include <WiFiUdp.h>            // Actualización por OTA
#include <Adafruit_MPU6050.h>   // Sensor MPU6050
#include <Adafruit_Sensor.h>    // Sensor MPU6050
#include <Wire.h>               // Sensor MPU6050
#include <Ticker.h>             // Temporizador funciones asincronas

#define ___DEBUG___

#ifdef ___DEBUG___
  #include <PriUint64.h>        // Imprimir en DEBUG variables const uint64_t
#endif

// Credenciales WIFI
const char* ssid = "TU_SSID";
const char* password = "TU_PASSWORD";
const char *hostname = "Cubo_Magico";

// Credenciales MQTT
#define MQTT_HOST IPAddress(192, 168, 1, 10)
#define MQTT_PORT 1883
const char* usuario = "TU_USER";
const char* pass_user = "TU_PASS";

// Topic MQTT
const char* topicEstado = "Casa/Cubo_Magico/Estado";
const char* topicLectura = "Casa/Cubo_Magico/Cara";
const char* topicBateria = "Casa/Cubo_Magico/Bateria";
const char* topicControl = "Casa/Cubo_Magico/Control";
static const char* topicTestamento = "Casa/Cubo_Magico/Testamento";

// Instancio objeto MQTT
AsyncMqttClient mqttClient;
uint16_t keepAlive = 120; // 2 minutos tiempo de conexión abierta

// Instancio objeto MPU
Adafruit_MPU6050 mpu;

// Programadores de eventos Ticker
Ticker ticker_bateria, ticker_zzz;

//  --------------  Variables del Proyecto ----------------------------------------

enum Exit_Deep_Sleep{     // Modos de salida del Deep Sleep -> touch, timer o Deep Sleep
    Exit_touch, Exit_timer, zzz
};
Exit_Deep_Sleep salida = zzz; // inicializo en Deep Sleep

const uint64_t uS_TO_S_FACTOR = 1000000;// Conversion factor -> micro seconds a seconds 
float TIME_TO_SLEEP = 43200;            // 12h x 3600 seg tiempo salir deep sleep 
touch_pad_t touchPin;                   // Determina el pin touch que despierta al ESP32
const uint16_t Threshold = 55;          // Límite para la actuación touch pin
const int sensor_pin = 4;               // pin interrupción Touch Pad 0 (GPIO4)
const int bateria_pin = 34;             // pin entrada analogica para leer capacidad bateria

float aX, aY, aZ;           // variables MPU6050 acelerometro ejes X, Y, Z
int cara = 0;               // cara del Cubo Mágico asociada a las ordenes 
int mem_cara = 0;           // memeria de la cara para hacer un número de mediciones antes de asignar una cara
int enviado = 0;            // variable que controla el envio de caras no repetidas
uint16_t valor_bateria;     // valor de carga bateria

//Variable de configuración a definir por el usuario
int muestras = 3;           // muestras para determinar la cara del Cubo Mágico
int acel = 6;               // valor de corte para decirir en que cara esta el Cubo Mágico
int t_reinit = 120;         // se va a dormir en X seg si no se envian caras/ordenes nuevas
int t_sample = 30;          // muestreo carga bateria del Cubo Mágico con despertar touch pin
int v_max = 2375;           // valor equivalentes al 100% -> 4.2 voltios

//////////////////////////////////// FUNCIONES ///////////////////////////////////////////////////////////////////////////////////////////////////

// callback llamada cuando se produce una alarma del sensor -> No SE USA
void callback(){

}

//Configuración del DEEP SLEEP 
void GoToZZZ(){
  if (salida == Exit_touch)
    mqttClient.publish(topicEstado, 0, false, "CUBO MAGICO se va a dormir");
  delay(1000);
  //Apago todo manualmente y se va a dormir
  mpu.enableCycle(false);
  mpu.enableSleep(true);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  btStop();
  esp_wifi_stop();
  esp_bt_controller_disable();

  //Habiliar despertador (TIMER) para salir de DEEP SLEEP
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  //Ajustes y configuración del touchpad
  touchAttachInterrupt(sensor_pin, callback, Threshold);
  //Habilitar Touchpad como forma para salir de DEEP SLEEP
  esp_sleep_enable_touchpad_wakeup();
  #ifdef ___DEBUG___
    Serial.println("Configurado ESP32 DEEP SLEEP despertando por Touchpad");
    Serial.print("Configurado ESP32 DEEP SLEEP despertando por TIMER cada ");
    Serial.print(PriUint64<DEC>(TIME_TO_SLEEP));
    Serial.println(" segundos");
    Serial.println("ZZZZZ ---- DEEP SLEEP ---- ZZZZZZ");
    delay(1000);
    Serial.flush();
  #endif
  //Ir a DEEP SLEEP
  esp_deep_sleep_start(); 
}

//    Funciones del Broker MQTT
//***********************************************************************************************************************

//Función para conectarse al Broker MQTT
void connectToMqtt() {
  
#ifdef ___DEBUG___
  Serial.println("********** connectToMqtt ************");
  Serial.println("Conectando CUBO MAGICO al Broker MQTT...");
#endif

  mqttClient.connect();
  
}

// Evento producido cuando se conecta al Broker
void onMqttConnect(bool sessionPresent) {

#ifdef ___DEBUG___
  Serial.println("********** onMqttConnect ************");
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
#endif

  mqttClient.setWill(topicTestamento, 2, false, "off");
  if (salida == Exit_touch)
    mqttClient.publish(topicEstado, 0, false, "CUBO MAGICO conectado al WIFI y al Broker MQTT. Esperando ordenes");
  delay(1000);

#ifdef ___DEBUG___
  uint16_t packetIdSub = mqttClient.subscribe(topicControl, 2);
  Serial.print("Suscrito a topicControl QoS 2, packetId: ");
  Serial.println(packetIdSub);
#else
  mqttClient.subscribe(topicControl, 2);
#endif

}

//Evento cuando se desconecta del Broker
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {

#ifdef ___DEBUG___
  Serial.println("********** onMqttDisconnect ************");
  Serial.println("CUBO MAGICO Desconectado del MQTT.....");
#endif

  if (WiFi.isConnected()) {
    connectToMqtt(); 
  }
  
}

//Gestión de Mensajes Suscritos MQTT ***************************************************************************************************
void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {

  String Str_topic = String(topic);
  String Str_payload;
  for (int i = 0; i < len; i++) {
    Str_payload+=(char)payload[i];
  }
 
#ifdef ___DEBUG___
  Serial.println("********** onMqttMessage ************");
  Serial.print("Message llegado [");
  Serial.print(Str_topic);
  Serial.print("]= "); Serial.println(Str_payload);
  Serial.print("  qos: ");  Serial.println(properties.qos);
  Serial.print("  dup: ");  Serial.println(properties.dup);
  Serial.print("  retain: ");  Serial.println(properties.retain);
#endif

  // Si se recibe un topic de Control NodeRed
  if (Str_topic==(String)topicControl){

    // Se reinicia Wemos NodeRed
    if (Str_payload == "R") {
      ESP.restart();
    } 

    // Retorna a DEEP SLEEP
    if (Str_payload == "ZZZ") {
      GoToZZZ();
    } 

    // Tiempo en segundos para salir del Deep Sleep (12h x 3600 seg = 43200 configuración inicial)
    if (Str_payload.substring(0, 14) == "TIME_TO_SLEEP=") {
      TIME_TO_SLEEP  = (Str_payload.substring(14, Str_payload.length())).toFloat();
      if (TIME_TO_SLEEP != 0)
        mqttClient.publish(topicEstado, 0, false, "comando TIME_TO_SLEEP OK, configurado nuevo tiempo de deep sleep");
      else
        mqttClient.publish(topicEstado, 0, false, "comando TIME_TO_SLEEP ERROR");
    }

    // Tiempo en segundos maximo sin ordenes antes de retornar a deep sleep
    if (Str_payload.substring(0, 9) == "t_reinit=") {
      t_reinit  = (Str_payload.substring(9, Str_payload.length())).toFloat();
      if (t_reinit != 0)
        mqttClient.publish(topicEstado, 0, false, "comando t_reinit OK, configurado nuevo tiempo máximo entre ordenes");
      else
        mqttClient.publish(topicEstado, 0, false, "comando t_reinit ERROR");
    }
  }

}

// Conectarse a red WIFI y al Broker MQTT
void connect_WIFI_MQTT(){
  IPAddress ip(192, 168, 1, 190); 
  IPAddress gateway(192,168,1,1);
  IPAddress subnet (255,255,255,0);
  IPAddress dns1(192,168,1,1);
  WiFi.config(ip,gateway,subnet,dns1);
  WiFi.persistent(false); // Desactiva la persistencia WiFi. El ESP32 no cargará ni guardará la 
                          // configuración de WiFi en la memoria flash.
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED){ 
    delay(100);  
    Serial.print('.'); 
  }

  #ifdef ___DEBUG___
    Serial.println("---------------- Conectado a la red WIFI ----------------");
    WiFi.printDiag(Serial);
    Serial.println("---------------------------------------------------------");
    Serial.flush();
  #endif
  connectToMqtt();
}

/*
Función que controla la razon o el metodo de salida de 
DEEP SLEEP (TIMER o Touchpad son los configurados en este proyecto)
*/
void wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch(wakeup_reason){
    case ESP_SLEEP_WAKEUP_EXT0 : 
      #ifdef ___DEBUG___
        Serial.println("Wakeup caused by external signal using RTC_IO"); 
      #endif
      break;
    case ESP_SLEEP_WAKEUP_EXT1 : 
      #ifdef ___DEBUG___
        Serial.println("Wakeup caused by external signal using RTC_CNTL"); 
      #endif
      break;
    case ESP_SLEEP_WAKEUP_TIMER : 
      #ifdef ___DEBUG___
        Serial.println("Wakeup caused by timer"); 
      #endif
      salida = Exit_timer;  // variable de control salida Deep Sleep
      break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : 
      #ifdef ___DEBUG___
        Serial.println("Wakeup caused by touchpad"); 
      #endif
      salida = Exit_touch;      // variable de control salida Deep Sleep
      touch_pad_intr_disable(); //Desabilita las interrupciones por touchpad
      mpu.enableCycle(false);   // activa MPU
      mpu.enableSleep(false);   // activa MPU
      break;
    case ESP_SLEEP_WAKEUP_ULP : 
      #ifdef ___DEBUG___
        Serial.println("Wakeup caused by ULP program"); 
      #endif
      break;
    default : 
      #ifdef ___DEBUG___
        Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); 
      #endif
      break;
  }
}

/*
Imprime el Touchpad que originó la salida de DEEP SLEEP
*/
void print_wakeup_touchpad(){
  touchPin = esp_sleep_get_touchpad_wakeup_status();
  switch(touchPin)  {
    case 0  : Serial.println("Touch detected on GPIO 4"); break;
    case 1  : Serial.println("Touch detected on GPIO 0"); break;
    case 2  : Serial.println("Touch detected on GPIO 2"); break;
    case 3  : Serial.println("Touch detected on GPIO 15"); break;
    case 4  : Serial.println("Touch detected on GPIO 13"); break;
    case 5  : Serial.println("Touch detected on GPIO 12"); break;
    case 6  : Serial.println("Touch detected on GPIO 14"); break;
    case 7  : Serial.println("Touch detected on GPIO 27"); break;
    case 8  : Serial.println("Touch detected on GPIO 33"); break;
    case 9  : Serial.println("Touch detected on GPIO 32"); break;
    default : Serial.println("Wakeup not by touchpad"); break;
  }
}

// Porcentaje de carga de la bateria del sensor
void Batery_Charge(){
    valor_bateria  = analogRead(bateria_pin);
    #ifdef ___DEBUG___
      Serial.println("% Carga Bateria = " + String(map(valor_bateria,0, v_max, 0, 100))); 
      Serial.flush();
    #endif
    mqttClient.publish(topicBateria, 0, false, (String(map(valor_bateria,0, v_max, 0, 100)).c_str()));
    delay(200);
}


//-------------------------------------------------------------------------------------------------------

void setup(){
  #ifdef ___DEBUG___
    Serial.begin(115200);
    delay(1000); 
  #endif

  Wire.begin(I2C_SDA, I2C_SCL);
  #ifdef ___DEBUG___
  Serial.println("Adafruit MPU6050 test!");
  #endif

  // Inicializo MPU6050
  if (!mpu.begin()) {
    #ifdef ___DEBUG___
      Serial.println("Failed to find MPU6050 chip");
    #endif
    while (1) {
      delay(10);
    }
  }
  else{
    #ifdef ___DEBUG___
      Serial.println("MPU6050 Found!");
    #endif
    // Se pasa a sleep mode para reducir el consumo
    mpu.enableCycle(false);
    mpu.enableSleep(true);
  }

#ifdef ___DEBUG___ 
  Serial.println("");
  Serial.print("CARA --> ");
  Serial.println(cara);
#endif
  delay(100);
  
  wakeup_reason();  //Controla el modo de salida de DEEP SLEEP ... TIMER o Touchpad
  #ifdef ___DEBUG___
    print_wakeup_touchpad();
  #endif
 
 #ifdef ___DEBUG___
    if (salida == zzz)
      Serial.println("Sensor en Deep Sleep ZZZ");
    if (salida == Exit_touch)
      Serial.println("Cubo Mágico despierto, se enviaran datos cara activa");
    if (salida == Exit_timer)
      Serial.println("Cubo Mágico envia datos de capaciad Bateria");
  #endif

  //Configuración DEEP SLEEP
  if (salida == zzz)
    GoToZZZ();
  #ifdef ___DEBUG___
    Serial.println("Saliendo de ---- DEEP SLEEP ----");
  #endif

  //Configuración MQTT
  mqttClient.setKeepAlive(keepAlive);
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.setCredentials(usuario, pass_user);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);

  //Conectamos WIFI y MQTT
  connect_WIFI_MQTT();

  //Inicializo OTA
  ArduinoOTA.setHostname(hostname); // Hostname OTA
  ArduinoOTA.begin();

  // Llamadas a funciones programadas
  ticker_bateria.attach(t_sample,Batery_Charge);

}

void loop(){
  
  ArduinoOTA.handle();  // Actualización código por OTA

  if (salida == Exit_timer){  // Se envia el estado de carga de la bateria
    for( int i=0; i< muestras; i++){
      Batery_Charge();
      delay(1000);
    }
    //Ir a DEEP SLEEP
    GoToZZZ(); 
  }

  if (salida == Exit_touch){    // Detecta y envía la posición del Cubo Mágico  
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    aX = a.acceleration.x;
    aY = a.acceleration.y;
    aZ = a.acceleration.z;

    if ((aX < 0) && (aY > 0) && (aZ > acel))
      cara = 1;
    if ((aX > 0 ) && (aY > 0) && (aZ < -acel))
      cara = 6;

    if ((aX > acel) && (aY < 0) && (aZ > 0))
      cara = 2;
    if ((aX < -acel ) && (aY > 0) && (aZ > 0))
      cara = 5;

    if ((aX > 0) && (aY > acel) && (aZ > 0))
      cara = 3;
    if ((aX < 0 ) && (aY < -acel) && (aZ > 0))
      cara = 4;

    if (cara == mem_cara){
      muestras--;
      mem_cara = cara;
    }
    else{
      muestras = 3;
      mem_cara = cara;
    }

    #ifdef ___DEBUG___
      Serial.println("");
      Serial.print("CARA --> ");
      Serial.println(cara);
      Serial.print("Acceleration X: ");
      Serial.print(aX);
      Serial.print(", Y: ");
      Serial.print(aY);
      Serial.print(", Z: ");
      Serial.print(aZ);
      Serial.println(" m/s^2");
      Serial.println("");
    #endif
    if (muestras == 0){
      muestras = 3;
      if (enviado != cara){
        #ifdef ___DEBUG___
          Serial.println("");
          Serial.print("CARA MQTT--> ");
          Serial.println(cara);
        #endif
        mqttClient.publish(topicLectura, 0, false, (String(cara)).c_str());
        delay(100);
        //Ir a DEEP SLEEP menos en cara 6 que se   
        //muestras el estado de carga bateria
        ticker_zzz.detach();
        if (cara != 6)
          ticker_zzz.attach(t_reinit,GoToZZZ);
      }
      enviado =cara;
    }
    delay(500);
  }

}
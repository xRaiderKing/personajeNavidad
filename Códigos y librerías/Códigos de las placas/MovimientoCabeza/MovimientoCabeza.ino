#include <Stepper.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

const int steps_per_rev = 2048;
const int pasos_180_grados = steps_per_rev / 2;
const int pasos_90_grados = steps_per_rev / 4;  // 180 grados
int echo = 26;
int trigger = 25;
int distancia = 0;

float temperatura = 0; // Señal que queremos publicar al broker /
float humedad = 0;
const int ledPin1 = 2;
const int ledPin2 = 4; // LedPin GPIO2
const int ledPin3 = 15; // LedPin GPIO15

// PWM

const int ledPin4 = 16; // LedPin GPIO16
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;

// SSID-Password de nuestro servidor a Internet
const char* ssid = "linksys"; // RUBY
const char* password = ""; // 43888776
// Dirección del MQTT Broker IP address
const char* mqtt_server = "192.168.1.100";

#define IN1 27
#define IN2 14
#define IN3 12
#define IN4 13

#define IN11 18
#define IN22 5
#define IN33 17
#define IN44 16

Stepper motor(steps_per_rev, IN1, IN3, IN2, IN4);
Stepper motorPata(steps_per_rev, IN11, IN33, IN22, IN44);

void setup() {
  Serial.begin(115200);
  motor.setSpeed(15);
  motorPata.setSpeed(15);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("Dirección IP: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Mensaje recibido en topic: ");
  Serial.print(topic);
  Serial.print(", Mensaje: ");
  String messageTemp;
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  if (String(topic) == "esp32/cabeza") {
    Serial.print("Cambio de salida: ");
    if (messageTemp == "on") {
      subirCabeza(360, 5);  // Girar el motor 2 giros completos de 360 grados
      bajarCabeza(360, 5);
    } else if (messageTemp == "off") {
      motor.step(0);
    }
  }else if (String(topic) == "esp32/pata") {
    Serial.print("Cambio de salida: ");
    if (messageTemp == "on") {
      moverMotorPata();  // Girar la pata 1 giro completo de 360 grados
    } else if (messageTemp == "off") {
      motorPata.step(0);
    }
  }
}

void hacerMov(){
  subirCabeza(360, 5);  // Girar el motor 2 giros completos de 360 grados
  bajarCabeza(360, 5);
  moverMotorPata();
}

void reconnect() {
  Serial.print("Intentando conexión MQTT...");
    if (client.connect("ESP8266Client")) {
      Serial.println("conectado");
      client.subscribe("esp32/cabeza");
      client.subscribe("esp32/pata");
    } else {
      Serial.print("Fallo, rc = ");
      Serial.print(client.state());
      Serial.println(" Intentándolo de nuevo en 5 segundos");
      hacerMov();
      delay(5000);
    }
}

void subirCabeza(int grados_por_giro, int giros){
  int pasos = grados_por_giro * steps_per_rev / 360 * giros;
  motor.step(pasos);
}

void bajarCabeza(int grados_por_giro, int giros) {
  int pasos = grados_por_giro * steps_per_rev / 360 * giros;
  motor.step(-pasos);  // Girar en la dirección opuesta para desenredar
}


void moverMotorPata(){
  motorPata.step(pasos_90_grados);
  delay(1000); 
  motorPata.step(-pasos_90_grados);
  delay(1000); 
}

long readUltrasonicDistance(int triggerPin, int echoPin) {
  pinMode(triggerPin, OUTPUT);
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  pinMode(echoPin, INPUT);
  return pulseIn(echoPin, HIGH) * 0.01723 / 2;  // Convertir a centímetros
}

void sensorProximidad() {
  distancia = readUltrasonicDistance(trigger, echo);
  if (distancia < 7) {
    Serial.print(distancia);
    subirCabeza(360, 5);  // Girar el motor 2 giros completos de 360 grados
    bajarCabeza(360, 5);
    moverMotorPata();  // Girar la pata 1 giro completo de 360 grados
  } else {
    motor.step(0);
    motorPata.step(0);
  }
  delay(20);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }else{
    hacerMov();
    sensorProximidad();
  }
  client.loop();
}

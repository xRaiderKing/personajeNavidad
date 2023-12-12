#include <Stepper.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
int led=14;

int buzzer = 15;

// SSID-Password de nuestro servidor a Internet
const char* ssid = "linksys"; // RUBY
const char* password = ""; // 43888776
// Dirección del MQTT Broker IP address
const char* mqtt_server = "192.168.1.100";

//Notas musicales
#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978
// ... (se omiten para brevedad, agregar todas las notas necesarias)

// Melodía de "Santa Claus is Coming to Town"
int santa_melody[] = {
  NOTE_G4,
  NOTE_E4, NOTE_F4, NOTE_G4, NOTE_G4, NOTE_G4,
  NOTE_A4, NOTE_B4, NOTE_C5, NOTE_C5, NOTE_C5,
  NOTE_E4, NOTE_F4, NOTE_G4, NOTE_G4, NOTE_G4,
  NOTE_A4, NOTE_G4, NOTE_F4, NOTE_F4,
  NOTE_E4, NOTE_G4, NOTE_C4, NOTE_E4,
  NOTE_D4, NOTE_F4, NOTE_B3,
  NOTE_C4
};

// Duración de cada nota en milisegundos
int santa_tempo[] = {
  8,
  8, 8, 4, 4, 4,
  8, 8, 4, 4, 4,
  8, 8, 4, 4, 4,
  8, 8, 4, 2,
  4, 4, 4, 4,
  4, 2, 4,
  1
};

// Jingle Bells

int jingle_melody[] = {
  NOTE_E5, NOTE_E5, NOTE_E5,
  NOTE_E5, NOTE_E5, NOTE_E5,
  NOTE_E5, NOTE_G5, NOTE_C5, NOTE_D5,
  NOTE_E5,
  NOTE_F5, NOTE_F5, NOTE_F5, NOTE_F5,
  NOTE_F5, NOTE_E5, NOTE_E5, NOTE_E5, NOTE_E5,
  NOTE_E5, NOTE_D5, NOTE_D5, NOTE_E5,
  NOTE_D5, NOTE_G5
};

int jingle_tempo[]  = {
  8, 8, 4,
  8, 8, 4,
  8, 8, 8, 8,
  2,
  8, 8, 8, 8,
  8,  8, 8, 16, 16,
  8, 8, 8, 8,
  4, 4
};

void setup() {
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void setup_wifi(){
  delay(10);
  Serial.println();
  Serial.print("Connectando a ");
  Serial.println(ssid); 
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); 
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Wifi conectado");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length){
  Serial.print("Mensaje recibido en topic: ");
  Serial.print(topic);
  Serial.print(", Message: ");
  String messageTemp;
  for(int i = 0; i < length; i++){
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  if(String(topic)=="esp32/musica"){
    Serial.print("Cambio de salida: ");
    if(messageTemp=="on"){
      playMelodyJingle();
      delay(1000);
      playMelodySanta();
    }else if(messageTemp=="off"){
      noTone(buzzer);
    }
  }else if(String(topic)=="esp32/leds"){
    Serial.print("Cambio de salida: ");
    if(messageTemp=="on"){
      digitalWrite(led, HIGH);
    }else if(messageTemp=="off"){
      digitalWrite(led, LOW);
    }
  }
}

void reconnect(){
  Serial.print("Intentando conexión MQTT...");
    if(client.connect("ESP8266Client")){
      Serial.println("conectado");
      client.subscribe("esp32/musica");
      client.subscribe("esp32/leds");
    }else{
      Serial.print("Fallo, rc = ");
      Serial.print(client.state());
      Serial.println(" Intentelo de nuevo en 5 seg");
      hacerMovimientoSn();
      delay(5000);
    }
}

void playMelodySanta() {
  for (int i = 0; i < sizeof(santa_melody) / sizeof(santa_melody[0]); i++) {
    int noteDuration = 2000 / santa_tempo[i];
    digitalWrite(led, HIGH);
    tone(buzzer, santa_melody[i], noteDuration);
    delay(noteDuration * 1.1);
    digitalWrite(led, LOW);
    noTone(buzzer);

    // Esperar un breve tiempo antes de la próxima nota
    delay(50);
  }
}

void playMelodyJingle() {
  for (int i = 0; i < sizeof(jingle_melody) / sizeof(jingle_melody[0]); i++) {
    int noteDuration = 2000 / jingle_tempo[i];
    digitalWrite(led, HIGH);
    tone(buzzer, jingle_melody[i], noteDuration);
    delay(noteDuration * 1.1);
    digitalWrite(led, LOW);
    noTone(buzzer);

    // Esperar un breve tiempo antes de la próxima nota
    delay(50);
  }
}

void hacerMovimientoSn(){
  playMelodyJingle();
  playMelodySanta();
}

void loop() {
  if(!client.connected()){
    reconnect();
  }else{
    hacerMovimientoSn();
  }
  client.loop();
}

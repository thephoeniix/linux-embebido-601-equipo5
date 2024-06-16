#include <DHT.h>
#include <Servo.h>
#include <Wire.h>
#include <RTClib.h>

#define DHTPIN 2
#define DHTTYPE DHT11
#define SOIL_MOISTURE_PIN A0
#define ULTRASONIC_TRIG_PIN 7
#define ULTRASONIC_ECHO_PIN 8
#define VENTILATION_SERVO_PIN 9
#define DOOR_SERVO_PIN 10
#define WATER_FLOW_SENSOR_PIN 3 // Pin para el sensor de flujo de agua
#define RED_PIN 4 // Pin para el LED rojo
#define GREEN_PIN 5 // Pin para el LED verde
#define BLUE_PIN 6 // Pin para el LED azul (no usado en este caso)

DHT dht(DHTPIN, DHTTYPE);
Servo ventilationServo;
Servo doorServo;
RTC_DS3231 rtc;

enum Mode { AUTOMATIC, MANUAL, SMART };
Mode currentMode = AUTOMATIC;

unsigned long previousMillis = 0;
const long interval = 60000;
volatile int pulseCount = 0;
float flowRate = 0;
String mensaje;
String received = "";
int manualWatering = 2;

void setup() {
  Serial.begin(9600);
  dht.begin();
  ventilationServo.attach(VENTILATION_SERVO_PIN);
  doorServo.attach(DOOR_SERVO_PIN);
  rtc.begin();
  pinMode(ULTRASONIC_TRIG_PIN, OUTPUT);
  pinMode(ULTRASONIC_ECHO_PIN, INPUT);
  pinMode(13, OUTPUT); // Pin for water pump (simulated with an LED)
  pinMode(WATER_FLOW_SENSOR_PIN, INPUT_PULLUP);
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(WATER_FLOW_SENSOR_PIN), pulseCounter, FALLING);
  ventilationServo.write(0); // Initially close ventilation
  doorServo.write(0); // Initially close the door

  while (!Serial.available()); // Wait for serial input before starting
  Serial.println("Initializing...");
}

void loop() {
  unsigned long currentMillis = millis();
  /*
  if (Serial.available() > 0) {
    char command = Serial.read();
    handleSerialCommand(command);
  }*/

  if (currentMode == AUTOMATIC && currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    handleAutomaticMode();
  } else if (currentMode == MANUAL) {
    handleManualMode();
  } else if (currentMode == SMART) {
    handleSmartMode();
  }

  handleVentilation();
  handleProximity();
  delay(100);
  
  String mensaje;
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Error al leer el sensor DHT!");
  } else {
    mensaje = "Caudal " + String(flowRate) + " L/min \n" + "Humedad: " + String(humidity) + " % \n" + "Temperatura: " + String(temperature) + " °C";
    Serial.println(mensaje);
  }

  received = "";
  if (Serial.available() > 0) {
    char c = Serial.read();
    received += c;
    delay(5);
  }
  if (received == "temp") {
    Serial.println(mensaje);
    mensaje = "";
    received = "";
  }

  if (received == "r") {
    handleManualMode();
    
  }

  delay(1000);
}

void pulseCounter() {
  pulseCount++;
}

void calculateFlowRate() {
  detachInterrupt(digitalPinToInterrupt(WATER_FLOW_SENSOR_PIN));
  flowRate = (pulseCount / 7.5); // Ajusta esta constante según la especificación del sensor ARD 370
  pulseCount = 0;
  attachInterrupt(digitalPinToInterrupt(WATER_FLOW_SENSOR_PIN), pulseCounter, FALLING);
}

void handleSerialCommand(char command) {
  switch (command) {
    case '1':
      currentMode = AUTOMATIC;
      break;
    case '2':
      currentMode = MANUAL;
      handleManualMode();
      break;
    case '3':
      currentMode = SMART;
      Serial.println("Modo: Inteligente");
      break;
    case 'h':
      printMenu();
      break;
    default:
      break;
  }
}

void printMenu() {
  // Optional: Implement menu print logic
}

void handleAutomaticMode() {
  DateTime now = rtc.now();
  if ((now.dayOfTheWeek() == 1 || now.dayOfTheWeek() == 3 || now.dayOfTheWeek() == 5) &&
      (now.hour() == 6 || now.hour() == 18)) {
    irrigate(10);
  }
}

void handleManualMode() {
  irrigate(1);
}

void handleSmartMode() {
  float soilMoisture = analogRead(SOIL_MOISTURE_PIN);
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (soilMoisture < 300 || h < 50 || t > 30) {
    irrigate(5);
  }
}

void handleVentilation() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (h > 50 || t > 30) {
    
    ventilationServo.write(90); // Abrir ventilación 90 grados
  } else {

    ventilationServo.write(0); // Cerrar ventilación
  }
}

void handleProximity() {
  long duration, distance;
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRASONIC_TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
  duration = pulseIn(ULTRASONIC_ECHO_PIN, HIGH);
  distance = (duration / 2) / 29.1;

  if (distance < 50) {
    doorServo.write(90); // Mover servo a 90 grados para abrir la puerta
    setLEDColor(GREEN_PIN); // Encender LED verde
    delay(10000); // Esperar 10 segundos
    // Verificar nuevamente la distancia
    digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(ULTRASONIC_TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
    duration = pulseIn(ULTRASONIC_ECHO_PIN, HIGH);
    distance = (duration / 2) / 29.1;
    if (distance >= 50) {
      doorServo.write(0); // Cerrar la puerta
      setLEDColor(RED_PIN); // Encender LED rojo
    } else {
      setLEDColor(GREEN_PIN); // Mantener LED verde
    }
  } else {
    doorServo.write(0); // Cerrar la puerta si no hay objeto
    setLEDColor(RED_PIN); // Encender LED rojo
  }
}

void setLEDColor(int pin) {
  digitalWrite(RED_PIN, LOW);
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(BLUE_PIN, LOW);
  digitalWrite(pin, HIGH);
}

void irrigate(int durationMinutes) {

  digitalWrite(13, HIGH); // Encender la bomba de agua (simulada con LED)
  delay(durationMinutes * 60000); // Esperar la duración especificada
  digitalWrite(13, LOW); // Apagar la bomba de agua
}

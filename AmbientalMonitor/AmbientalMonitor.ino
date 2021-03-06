/*
 * Ambiental Monitor using luminosity and temperature sensors.
 *
 *  Optionals:
 *  - Convert ldr reading into Lux (necessary to make a chart table with lux readings)
*/

#include <ArduinoJson.h>

#define referencePotencial 5.0 // Power source to LDR
#define miliToVolts 100.0   // convert mV to V (Volts)
#define timeInterval 1.0/12.0 // Time interval in minutes - minimum 1.0/30.0, due reading time
#define millisToMinute 1000*60

int ledCommunicationPin = 7;
int ledLuminosityPin = 10;
int lightSensorPin = A5;
int temperatureSensorPin = A0;

void setup() {
  Serial.begin(9600);
  while (!Serial) ;

  pinMode(ledCommunicationPin, OUTPUT);
  pinMode(ledLuminosityPin, OUTPUT);
}

void loop() {
  int ldrReading = 0;
  int luminosity = 0;
  int celsiusTemp = 0;
  int fahrenheitTemp = 0;

  temperatureReading(celsiusTemp, fahrenheitTemp);

  luminosityReading(ldrReading, luminosity);
  luminosityLogging(ldrReading, luminosity);

  JsonObject& json = buildJson(celsiusTemp, ldrReading);

  sendData(json);

  delay(calculateDelayTime());
}

/*
 * Leitura do sensor de temperatura LM35z.
 *
 * Faz 8 medidas para melhor precisão e calcula a média (ao longo de 800ms).
 * Converte para Farenheit.
 */
void temperatureReading(int& celsius, int& farenheit) {
  int samples[8];

  for (int i = 0; i <= 7; i++) {
    samples[i] = ( referencePotencial * analogRead(temperatureSensorPin) * miliToVolts) / 1024.0;
    //A cada leitura, incrementa o valor da variavel celsiusTemp
    celsius = celsius + samples[i];
    delay(100);
  }
  celsius = celsius / 8.0;
  farenheit = (celsius * 9) / 5 + 32;
}

/*
 * LDR reading. (0~1023)
 *
 * To set a scale in LUX, it would be needed a LUX sensor to create a correlation between tensions
 * and LUX values (which grows in logarithmic scale).
 */
void luminosityReading(int& state, int& luminosity) {
  state = analogRead(lightSensorPin);
  luminosity = map(state, 0, 1023, 0, 255);
}

/*
 * Set the LDR reading value into a common LED connected to Arduino analog pin.
 */
void luminosityLogging(int& state, int& luminosity) {
  if (state > 800)
    analogWrite(ledLuminosityPin, 1023);
  else if (state < 150)
    analogWrite(ledLuminosityPin, 0);
  else
    analogWrite(ledLuminosityPin, luminosity);
}

/*
 * Serialize the parameters into a JSON file.
 */
JsonObject& buildJson(int& celsius, int& luminosity) {
  StaticJsonBuffer<300> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  root["temperature"] = celsius;
  root["luminosity"] = luminosity;

  return root;
}

/*
 * Send Json object through serial port.
 * - indicates flashing Commnunication Led
 */
void sendData(JsonObject& json) {
  digitalWrite(ledCommunicationPin, HIGH);
  json.printTo(Serial);
  Serial.println();
  delay(50);
  digitalWrite(ledCommunicationPin, LOW);
}

long calculateDelayTime() {
  return (long) (timeInterval * millisToMinute) - millis() % (long)(timeInterval * millisToMinute);
}

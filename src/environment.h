#ifndef _ENVIRONMENT_H_
#define _ENVIRONMENT_H_

Adafruit_SHT31 sht30 = Adafruit_SHT31();

void publishEnvironmentTemperature();
void publishEnvironmentHumidity();


void publishEnvironmentTemperature() {
    publishMessage(diagnostics_environment_temperature.c_str(), String(sht30.readTemperature()), true, true);
}

void publishEnvironmentHumidity() {
    publishMessage(diagnostics_environment_humidity.c_str(), String(sht30.readHumidity()), true, true);
}

#endif
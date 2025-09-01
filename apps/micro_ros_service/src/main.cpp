#include <Arduino.h>
#include <MicroRosSrv.h>

void setup() {
    Serial.begin(115200);

    pinMode(23, OUTPUT);
    digitalWrite(23, HIGH);
    delay(500);
    digitalWrite(23, LOW);

    controllerSrv.begin();
}

void loop() {    

}
// Back motors: both rear wheels forward for RUN_MS from power-up, then stop. Nothing else.
//
// Back L298N: channel B drives back-left, channel A drives back-right (its outputs are wired swapped).

#include <Arduino.h>


// %% Config
constexpr uint32_t RUN_MS = 5000;


// %% Motors
struct Motor {
    uint8_t en;       // PWM speed pin
    uint8_t in1;      // direction pins
    uint8_t in2;
    bool    invert;   // flip if the motor spins the wrong way
};

const Motor BACK_LEFT  = {10, A2, A3, true};
const Motor BACK_RIGHT = {9,  A0, A1, false};


// %% Helpers
void setMotor(const Motor& m, int speed) {
    if (m.invert) speed = -speed;
    digitalWrite(m.in1, speed > 0 ? HIGH : LOW);
    digitalWrite(m.in2, speed < 0 ? HIGH : LOW);
    analogWrite(m.en, abs(speed));
}


// %% Setup
void setup() {
    const Motor* motors[] = {&BACK_LEFT, &BACK_RIGHT};
    for (const Motor* m : motors) {
        pinMode(m->en,  OUTPUT);
        pinMode(m->in1, OUTPUT);
        pinMode(m->in2, OUTPUT);
    }
    setMotor(BACK_LEFT,  255);
    setMotor(BACK_RIGHT, 255);
    delay(RUN_MS);
    setMotor(BACK_LEFT,  0);
    setMotor(BACK_RIGHT, 0);
}


// %% Loop
void loop() {}

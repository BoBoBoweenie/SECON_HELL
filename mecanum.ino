#include <Bluepad32.h>

// --- MOTOR PIN DEFINITIONS (2 wires per motor) ---
// Front Left Motor
const int FL_1 = 2;
const int FL_2 = 4;

// Front Right Motor
const int FR_1 = 33;
const int FR_2 = 25;

// Back Left Motor
const int BL_1 = 16;
const int BL_2 = 17;

// Back Right Motor
const int BR_1 = 26;
const int BR_2 = 27;

ControllerPtr myControllers[BP32_MAX_CONTROLLERS];

// --- SENSITIVITY MULTIPLIERS ---
float translationSensitivity = 1.0; // Scales throttle and strafe (0.2 to 2.0)
float rotationSensitivity = 1.0;    // Scales rotation (0.2 to 2.0)
unsigned long lastDpadPress = 0;    // Debounce timer for D-pad inputs

void onConnectedController(ControllerPtr ctl) {
    for (int i = 0; i < BP32_MAX_CONTROLLERS; i++) {
        if (myControllers[i] == nullptr) {
            myControllers[i] = ctl;
            Serial.print("Controller connected at index: ");
            Serial.println(i);
            return;
        }
    }
}

void onDisconnectedController(ControllerPtr ctl) {
    for (int i = 0; i < BP32_MAX_CONTROLLERS; i++) {
        if (myControllers[i] == ctl) {
            myControllers[i] = nullptr;
            Serial.println("Controller disconnected!");
            stopMotors();
            return;
        }
    }
}

// Helper function to drive an individual motor with a 2-wire setup (-255 to 255 speed)
void driveMotor(int pin1, int pin2, int speed) {
    speed = constrain(speed, -255, 255);
    if (speed > 0) {
        analogWrite(pin1, speed);
        analogWrite(pin2, 0);
    } else if (speed < 0) {
        analogWrite(pin1, 0);
        analogWrite(pin2, -speed);
    } else {
        analogWrite(pin1, 0);
        analogWrite(pin2, 0);
    }
}

void stopMotors() {
    driveMotor(FL_1, FL_2, 0);
    driveMotor(FR_1, FR_2, 0);
    driveMotor(BL_1, BL_2, 0);
    driveMotor(BR_1, BR_2, 0);
}

void setup() {
    Serial.begin(115200);

    // Initialize motor pins as outputs
    pinMode(FL_1, OUTPUT); 
    pinMode(FL_2, OUTPUT);
    
    pinMode(FR_1, OUTPUT); 
    pinMode(FR_2, OUTPUT);
    
    pinMode(BL_1, OUTPUT); 
    pinMode(BL_2, OUTPUT);
    
    pinMode(BR_1, OUTPUT); 
    pinMode(BR_2, OUTPUT);

    stopMotors();

    BP32.setup(&onConnectedController, &onDisconnectedController);
    BP32.forgetBluetoothKeys();
    
    Serial.println("Robot Ready. Connect your controller.");
    Serial.println("Use D-Pad Up/Down for Translation Sensitivity, Left/Right for Rotation Sensitivity.");
}

void loop() {
    BP32.update();

    bool controllerActive = false;

    for (int i = 0; i < BP32_MAX_CONTROLLERS; i++) {
        ControllerPtr ctl = myControllers[i];
        if (ctl && ctl->isConnected()) {
            controllerActive = true;

            // --- D-PAD SENSITIVITY ADJUSTMENT (with simple debounce) ---
            uint8_t dpad = ctl->dpad();
            if (millis() - lastDpadPress > 250) { // 250ms debounce
                if (dpad & DPAD_UP) {
                    translationSensitivity += 0.1;
                    if (translationSensitivity > 2.0) translationSensitivity = 2.0;
                    Serial.print("Translation Sensitivity: ");
                    Serial.println(translationSensitivity);
                    lastDpadPress = millis();
                } else if (dpad & DPAD_DOWN) {
                    translationSensitivity -= 0.1;
                    if (translationSensitivity < 0.2) translationSensitivity = 0.2;
                    Serial.print("Translation Sensitivity: ");
                    Serial.println(translationSensitivity);
                    lastDpadPress = millis();
                }

                if (dpad & DPAD_RIGHT) {
                    rotationSensitivity += 0.1;
                    if (rotationSensitivity > 2.0) rotationSensitivity = 2.0;
                    Serial.print("Rotation Sensitivity: ");
                    Serial.println(rotationSensitivity);
                    lastDpadPress = millis();
                } else if (dpad & DPAD_LEFT) {
                    rotationSensitivity -= 0.1;
                    if (rotationSensitivity < 0.2) rotationSensitivity = 0.2;
                    Serial.print("Rotation Sensitivity: ");
                    Serial.println(rotationSensitivity);
                    lastDpadPress = millis();
                }
            }

            // Read joystick axes (Bluepad32 axes typically range from -511 to 511)
            int axisX = ctl->axisX();   // Left stick X (Strafe)
            int axisY = ctl->axisY();   // Left stick Y (Forward/Backward)
            int axisRX = ctl->axisRX(); // Right stick X (Rotation)

            // Apply a small deadzone to prevent joystick drift
            if (abs(axisX) < 20) axisX = 0;
            if (abs(axisY) < 20) axisY = 0;
            if (abs(axisRX) < 20) axisRX = 0;

            // Map inputs from joystick range to motor speed range (-255 to 255)
            // Note: Invert Y axis because pushing up yields negative values on gamepads
            float strafe = map(axisX, -511, 511, -255, 255) * translationSensitivity;
            float throttle = map(axisY, -511, 511, 255, -255) * translationSensitivity; 
            float rotation = map(axisRX, -511, 511, -255, 255) * rotationSensitivity;

            // Mecanum Kinematics Mixing Formula
            float fl = throttle + strafe + rotation;
            float fr = throttle - strafe - rotation;
            float bl = throttle - strafe + rotation;
            float br = throttle + strafe - rotation;

            // Drive the 4 motors
            driveMotor(FL_1, FL_2, (int)fl);
            driveMotor(FR_1, FR_2, (int)fr);
            driveMotor(BL_1, BL_2, (int)bl);
            driveMotor(BR_1, BR_2, (int)br);
        }
    }

    if (!controllerActive) {
        stopMotors();
    }

    delay(15); // Small loop refresh delay
}
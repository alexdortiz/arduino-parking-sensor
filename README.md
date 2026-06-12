# Ultrasonic Parking Sensor

Real-time parking assist system built on an Arduino Mega. Measures distance with an HC-SR04 ultrasonic sensor, displays it live on an LCD1602, and escalates alerts with LEDs and a buzzer as an object approaches — the same concept used in vehicle backup sensors.

<img width="935" height="1247" alt="image" src="https://github.com/user-attachments/assets/2e7ef86b-f08e-4531-bd6e-315187f6c856" />


## Features

- Continuous distance measurement (multiple readings per second)
- Live distance readout on LCD1602 ("Dist: 141 cm")
- Three-state alert logic: ALL CLEAR (green LED) → warning (red LED) → close range (red LED + buzzer)
- Noise filtering to reject spurious echo readings (sudden 0 cm / out-of-range values)
- Non-blocking loop structure so the buzzer never stalls sensor timing

## Hardware

| Component | Purpose |
|---|---|
| Arduino Mega 2560 (ELEGOO) | Microcontroller |
| HC-SR04 | Ultrasonic distance sensor |
| LCD1602 + potentiometer | Distance display + contrast |
| Green & red LEDs + resistors | Visual proximity indicators |
| Piezo buzzer | Audible close-range alert |
| Breadboard + jumper wires | Prototyping |

## How It Works

1. The Mega triggers the HC-SR04 and times the echo pulse
2. Echo time is converted to centimeters; invalid readings are filtered out before reaching display logic
3. Distance is compared against thresholds to set the LED/buzzer state
4. LCD updates continuously without blocking the measurement loop

## Running It

1. Wire components per the pin assignments at the top of the sketch
2. Open the `.ino` in Arduino IDE, select **Arduino Mega 2560** and your COM port
3. Upload — the LCD shows live distance immediately

## What I Learned

Sensor pulse timing, filtering noisy analog-world data, and structuring embedded code around non-blocking loops so multiple outputs (LCD, LEDs, buzzer) stay responsive.

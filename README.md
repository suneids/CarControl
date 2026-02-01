<h2 align="center"> Remote Controlled Car Mk2.5 </h2>
  
A high-performance 4-wheel robotic platform controlled via a custom gesture-based glove or mobile app.


<h2 align="center">🚀 Key Features</h2>

- **Bare Metal Firmware**: Written entirely using CMSIS and direct register access for maximum performance and minimum memory footprint.
- **Advanced Power Path**: Replaced standard linear regulators with a custom-integrated MP2315 Buck Converter for high-efficiency power delivery.
- **Dual-Mode Input**: Supports seamless switching between Mobile App (Bluetooth) and Empower Glove (Custom Radio) interfaces.
- **Data Integrity**: Custom packet protocol with XOR checksum validation to ensure stable control in noisy RF environments.


<h2 align="center">🛠 Technical Stack </h2>

## Hardware
- **MCU**: STM32F103C8T6 (Cortex-M3).
- **Radio**: HC-12 (433MHz Long Range) / HC-05 (Bluetooth Classic).
- **Power System**: 2S/3S Li-Po input → Integrated MP2315 Buck Converter (3.3V Output).
- **Drive System**: Dual DC Motors (H-Bridge) + Dual Steering Servos.

## Software Architecture ([CustomHAL](https://github.com/suneids/HAL_STM32F103C6T6))
Custom-built drivers for core peripherals:

- **GPIO**: Low-latency pin configuration and atomic bit-toggling.
- **TIM (PWM)**: High-frequency (20kHz) motor control and precise servo positioning.
- **USART**: Interrupt-driven serial communication with UART for auxiliary inputs.
- **SysTick**: Microsecond-accurate timing for system orchestration.

<h2 align="center">📡 Communication Protocol</h2>

The system utilizes a 4-byte packet structure for real-time control:

| Byte | Field | Description |
| ---- | ----  | ----        |
| 0 | Header | 0xFE |
| 1 | DegSteering Angle  | 0-180 |
| 2 | SpeedSigned Velocity |  -100 to 100 |
| 3 | XORChecksum |  (uint8_t)Deg ^ (uint8_t)Speed |

**Failsafe Logic**: Packets failing the XOR validation are discarded immediately to prevent erratic movement caused by signal interference.

<h2 align="center">🏗 Firmware Logic</h2>

The firmware operates as a State Machine:

- **Input Polling**: Monitors mode-switch interrupts 
- **Packet Parsing**: Validates incoming serial streams via header detection and checksum verification.
- **Kinematics & Normalization**: Translates raw degree/speed data into PWM duty cycles, incorporating mechanical calibration (FixTurn logic).
- **Execution**: Direct register updates to TIMx->CCR registers for instantaneous response.

<h2 align="center">Future Roadmap</h2>

- [ ] 4-Layer PCB Migration: Implementing a dedicated ground plane to suppress switching noise from the Buck converter.
- [ ] Watchdog Implementation: Auto-stop safety feature if no valid packets are received within 500ms.
- [ ] Inertial Navigation: Integrating magnetometer data for "Compass Mode" heading hold.

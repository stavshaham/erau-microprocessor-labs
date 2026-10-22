# ERAU Prescott - Microprocessor Systems Labs

This repository contains lab assignments, hardware integrations, and firmware projects developed for the **Microprocessor Systems Laboratory** course at **Embry-Riddle Aeronautical University, Prescott**. 

The projects focus on low-level programming, register-level configuration, and peripheral management using the ARM Cortex-M4 architecture.

---

## 🛠️ Tech Stack & Hardware

* **Languages:** C, Assembly
* **Microcontroller:** Tiva C Series TM4C123G Development Board (ARM Cortex-M4)
* **Development Environment:** IAR Embedded Workbench
* **Test & Measurement Equipment:** Oscilloscopes, Digital Multimeters
* **Peripherals & Protocols:** GPIO, UART, Analog-to-Digital Converter (ADC), Timers & Interrupts

---

## 📁 Repository Structure

```text
.
├── myHello/                  # Lab 1 - Introduction to IAR workspace and GPIO toggle
├── my_uart_echo/             # Lab 2 - Serial communication and baud rate configuration
├── my_ADC_lab3/              # Lab 3 - Analog signal sampling and conversion
├── myTimers/                 # Lab 4 - Hardware timer configuration and ISR handling
├── myTimersAssembly/         # Lab 5 - Hardware timer configuration and ISR handling using assembly
└── common/                   # Shared header files, startup files, and driver libraries
```

## 🔬 Lab Overviews

### Lab 1: IDE & Hello World with Blinky
* **Objective:** Set up the IAR Embedded Workbench toolchain, configure project settings, and control GPIO pins to blink onboard LEDs.
* **Key Concepts:** Memory-mapped register access, clock gating, and digital output configuration.

### Lab 2: UART Communication
* **Objective:** Establish asynchronous serial communication between the TM4C123G board and a host terminal using UART peripherals.
* **Key Concepts:** Baud rate generation, data framing, transmit/receive FIFO management, and polling-based serial I/O.

### Lab 3: Analog-to-Digital Converters (ADC)
* **Objective:** Configure the internal ADC module to sample analog voltage inputs and process the digital results.
* **Key Concepts:** Sample sequencers, reference voltages, trigger sources, and digital data scaling.

### Lab 4: Timers (Interrupts)
* **Objective:** Implement General-Purpose Timer blocks to generate precise time delays and service asynchronous events via Interrupt Service Routines (ISRs).
* **Key Concepts:** Periodic timer mode, reload registers, NVIC (Nested Vectored Interrupt Controller) configuration, and ISR priority.

### Lab 5: Assembly Language (ISR)
* **Objective:** Optimizing the ISR from Lab 4 using assembly, and getting rid of branching, MOVS, and ADDS.
* **Key Concepts:** Implement assembly knowledge learned in class on a microcontroller.
## 🚀 Getting Started

### Prerequisites
* Install **IAR Embedded Workbench for ARM**.
* Install the Texas Instruments TivaWare driver library for the TM4C series.
* Connect your TM4C123G Development Board to your PC via USB (In-Circuit Debug Interface).

### Building and Flashing
1. Clone the repository:
   ```bash
   git clone https://github.com/stavshaham/erau-microprocessor-labs.git
   ```
2. Open the desired lab `.eww` workspace file in IAR Embedded Workbench.
3. Set the desired lab project to Active (Right-Click --> Set as Active).
4. Clean the build, then build (`Make`) the program.
5. Download the project to the microcontroller (`Project --> Download --> Download Active Application`) to flash the code to the TM4C123G microcontroller and step through execution.
6. Click the reset button on the microcontroller.

---

## 👤 Author

* **Stav Shaham**
* Software Engineering | Embry-Riddle Aeronautical University, Prescott (Class of 2028)

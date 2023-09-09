# Mobile Temperature Logger (MTL)

The Mobile Temperature Logger (MTL) is an open-source embedded system designed for monitoring temperature using a battery-powered USB temperature logger. This repository contains all the necessary files and resources to build, program, and customize your MTL device.

<img src="https://github.com/bin2ai/mobile-temperature-logger/blob/main/hw/mtl_assembled.png" width="100">       <img src="https://github.com/bin2ai/mobile-temperature-logger/blob/main/sw/images/Capture.PNG" width = "100">
## Table of Contents

- [Mobile Temperature Logger (MTL)](#mobile-temperature-logger-mtl)
  - [Table of Contents](#table-of-contents)
  - [Features](#features)
  - [Getting Started](#getting-started)
    - [Hardware](#hardware)
    - [Software](#software)
    - [Firmware](#firmware)
  - [Usage](#usage)
  - [Contributing](#contributing)
  - [License](#license)

## Features

- Battery-powered USB temperature logging
- ATmega32U4-based firmware for data collection
- GUI software for data visualization
- Open-source hardware design files
- Customizable and extendable for various applications

## Getting Started

### Embedded Hardware

The MTL hardware design files are available in this repository, including:

- Schematic (SCH)
- Bill of Materials (BOM)
- PCB Layout (PCB)

You can use these files to manufacture your own MTL hardware. Make sure to review the schematic, source components, and order the PCB.

### MCU Firmware

The MTL firmware is written in C++ and is designed to be flashed onto the ATmega32U4 microcontroller. To get started with the firmware:

1. Navigate to the 'fw' directory.
2. Open the firmware project in your preferred development environment.
3. Compile the firmware and flash it onto the ATmega32U4 microcontroller.

### GUI Software

The MTL GUI software allows you to visualize and manage temperature data collected by your MTL device. To get started with the GUI software:

1. Navigate to the 'sw' directory.
2. Install the required dependencies (provide a list in your 'README' if necessary).
3. Run the GUI software using the provided instructions.

## Usage

1. Connect your MTL device to a USB port or power it using the battery.
2. Launch the GUI software and connect to your MTL device.
3. Monitor and analyze temperature data in real-time or export it for further analysis.

For detailed usage instructions and additional features, refer to the user documentation available in the 'docs' directory.

## Contributing

We welcome contributions from the open-source community! If you want to contribute to the MTL project, please follow these steps:

1. Fork the repository.
2. Create a new branch for your feature or bug fix.
3. Make your changes and commit them with clear messages.
4. Create a pull request with a detailed description of your changes.

We appreciate your contributions!

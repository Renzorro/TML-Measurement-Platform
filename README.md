# TML-Measurement-Platform
This repository contains the code related to the "Weight Distribution Clinical Measurement Platform" designed for The Motion Lab at the Vancouver Children's Hospital. It contains the firmware for the ESP32-S3 Microcontroller, and the computer GUI used to communicate with it. This repository is one part of our UBC Mechanical Engineering Capstone Project.

## desktop-application-exe
Contains the .exe file and the .ico file needed to run the desktop application without python.

## desktop-application-python
Contains the python files needed to run the desktop application to control the device. Run **main.pyw** to open the application. Make sure this file is in the same directory as all the other files in the "desktop-application" foler, aside from the exe file. Alternatively, the exe file cna be ran to open the application, but the exe file has to be in the same directory as the icon.ico file too.

## microcontroller-firmware
Contains the .ino file uploaded to the ESP32S3 using the Arduino IDE, and the libraries included in the Arduino IDE which are needed before uploading.

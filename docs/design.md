# BatteruEmulator v6.x SWAD

## Introduction
The v6.x architecture is designed to provide better abstraction and priority in the core and auxiliary functionality.

## System overview
All functionality runs on 1 core. The Arduino loop() function is the IDLE task, and mostly handles initializtion and the watchdog. In addition, there is a Core task that handles battery-side and inverter-side data and communication. Lastly, there is a task dedicated to wireless communication.

## Safety
Safety is central for this type of application where mishandling of the battery can lead to fires. To cover as much as possible, a simplified FMEA can be used.

### Failure Modes and Effects Analysis (FMEA)
...

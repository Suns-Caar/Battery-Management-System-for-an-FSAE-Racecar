# Battery-Management-System-for-an-FSAE-Racecar

BMS is responsible for monitoring, controlling, and optimizing the performance, safety, and
longevity of the Accumulator.
The Design is based on Analog Devices's LTC Series LTC6813 Demo Boards(i.e DC2350B).
LTC6813 - [https://www.analog.com/en/products/ltc6813-1.html](url)
DC2350B- [https://www.analog.com/en/design-center/evaluation-hardware-and-software/evaluation-boards-kits/dc2350b.html](url)
Tasks of AMS
1. Establish SPI Communication with the IsoSPI-SPI Transciever and monitor the cell
voltage from 107 cell taps from 6 LTC6813 slaves.
2. Establish CAN Communication with the Thermistor Modules and monitor 160
thermistors over CAN
3. Perform a basic SoC estimation based on the Voltage method by using the cell voltages
and their discharge curve. Estimation methods like coulomb counting cant be used
since the LTC demoboards dont have any provision to measure cell currents.
4. Log the cell voltages and the temperatures using the onboard SD card. This logging is
seperate from the Logging in the DAQ system making it completely independent.
5. Send a periodic CAN message outside the Accumulator using the 2nd CAN controller
of Teensy. This CAN message will have Highest, Lowest, Average Cell voltages and
Cell temps of the pack for use in displaying on the Display.
6. Motor Controller Derating is performed via a CAN message to the PM100DZ as it
senses the pack current using a current sensor.
7. Send an output to the AMS Latch PCB when Cell Voltages or Temps exceed their
limits.
To Perform all these tasks, we chose to use Teensy 4.1 as the AMS master due to its smaller
form factor, High CPU speed, 3 CAN controllers and onboard SD Card all at a cheaper cost.
The above tasks will be performed over FREERTOS.

For Thermistor CAN communication to the AMS, the module sends out a continous CAN
message with ID-1838F380 whose first byte iterates over the thermistors from 0-80 which will
be read by the AMS in a for loop and assigned to an array of size 80 which when fills up will
be displayed on the Serial Monitor while also being logged. Since these two task of reading
CAN and logging will use the critical section of the code, Mutexes or Semaphores will be used.
The same step above will be executed for the Module #2

![image](https://github.com/Suns-Caar/Battery-Management-System-for-an-FSAE-Racecar/assets/73470491/d723e69a-bf36-4046-bdb6-fdabad217f43)


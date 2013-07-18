Kartduino
=========

FAQ:
=========

Q: Kartduino, clever name. Does it mean anything?

A: Initial aim was for a simple DIY ECU for go-karts. The important part of that being SIMPLE! The code is written with simplicity above all else in mind and all areas are well commented. Where code is complex or low level, a high level pseudo code example is given to explain what is being performed. 

========================================================================

Q: Arduino ECU,pffft, heard THAT before. Does this one actually work?

A: Nope, well, not as of June 2013. That said, the software IS largely complete and many of the subsystems are well tested. A reference hardware setup is currently being put together for full testing

========================================================================

Q: So what can it do?

A: Initial plan is for injection and ignition control on single cylinder 4 stroke engines. 
Features:
* 8x8 maps with interpolation
* Support for missing tooth crank wheels up to 36-1 with hall effect sensor
* Alpha-N or Speed Density load control
* High-Z injector hardware

Phase 2 will include:
* Batch support for up to 4 cylinder engines
* Speed Density (MAP) and mixed mode load calculations
* 2 stroke engine support

========================================================================

Q: Target platform?

A: Originally it was the Freetronics Leostick (http://www.freetronics.com/collections/arduino/products/leostick) with the aim of making a teeny tiny ECU. This Leonardo compatible board is suitable for a single cylinder applications but lacks the hardware timers do multi-cylinder engines. For Phase 2 (Including up to 4 cylinders) an Arduino Mega (Or other ATmega1280 / ATmega2560 powered SKU) will be required

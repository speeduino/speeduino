Speeduino
=========

FAQ:
=========

Q: Speeduino, didn't you used to be called something else?

A: Initial aim was for a simple DIY ECU for go-karts and the original project title was kartduino. As it became apparent that we wanted to extend support to larger vehicles, the name became a bit misleading.
The important ideas though remain the same and the priority is still SIMPLICITY! The code is written with simplicity above all else in mind and all areas are well commented. The project can be loaded directly into the Arduino IDE and compiled with only a single additional library needing to be added. Where code is complex or low level, a high level pseudo code example is given to explain what is being performed. 

========================================================================

Q: Arduino ECU,pffft, heard THAT before. Does this one actually work?

A: Yep! Single cylinder engines are well tested and working as of Nov 2013 and multi-cylinder support is in testing. Please see the dev roadmap (https://github.com/noisymime/speeduino/wiki/Development-Roadmap) for the current status of in progress work

========================================================================

Q: So what can it do?

A: Initial plan is for injection and ignition control on single cylinder 4 stroke engines. 
Features:
* 8x8 maps with interpolation
* Support for missing tooth crank wheels up to 36-1 with hall effect sensor
* Alpha-N or Speed Density load control
* High-Z injector hardware
* Warmup enrichment
* Acceleration enrichment
* Compatible with Tuner studio (http://tunerstudio.com/) for tuning

Phase 2 will include:
* Batch support for up to 4 cylinder engines
* 2 stroke engine support
* Closed loop EGO / O2 
* Autotune with TunerStudio and wideband O2

========================================================================

Q: Target platform?

A: Arduino Mega (Or other ATmega1280 / ATmega2560 powered SKU) will be required. Standard arduino models are not suitable for multi-cylinder engines due to their lack of 16-bit timers. The shield board has been designed around the Mega and it is not expected that there will be a non-Mega variant. 

========================================================================

Q: I'm trying to get started or need some general help. What should I do?

A: Support is all through the forum at: http://speeduino.com/forum

========================================================================

Q: I must know more! How can I contact you?

A: Come and visit the forum : http://speeduino.com/forum

If you still need some more info, feel free to drop me a line: noisymime (AT) gmail (dot) com

# digitalIOPerformance.h

digitalIOPerformance.h is a single-file Arduino library (header file)
that adds higher-performance digital I/O functions to Arduino
programs.

# Quick Start

* Copy the "digitalIOPerformance" directory to your [Arduino libraries folder](http://arduino.cc/en/Guide/Libraries).

* Add _#include &quot;digitalIOPerformance.h&quot;_ near the top of your sketch.

* Done! When you recompile, performance of digitalRead/digitalWrite &
  pinMode should be substantially faster in most cases. However,
  functionality should be otherwise identical to the original Arduino
  functions.

* Your sketch's compiled size may also go down (depending on how much
  digital I/O you do.)

## What Becomes Faster?

Any digital I/O when the pin number is known at compile time:

    const int led_pin = 13;
    digitalWrite(led_pin, HIGH);  // <-- gets ~30x faster

    #define RELAY_PIN 11
    digitalWrite(RELAY_PIN, x>2); // <-- also ~30x faster

Not the case where the pin number isn't known at compile time:

    int my_pin = 4; // <-- note this is a variable, no 'const' marker!
    
    void loop() {
       digitalWrite(my_pin, LOW); // <-- same speed as normal Arduino
    }

## Option: More performance, smaller compiled size, on PWM Enabled pins

The Arduino's digital I/O functions deal with the
possibility that a pin is used for PWM output with
[analogWrite()](http://arduino.cc/en/Tutorial/PWM), and then the same
pin gets used again later for normal digital output with
digitalWrite(). Something like this:

    analogWrite(MY_PIN, 120);
    // ... do some stuff
    digitalWrite(MY_PIN, LOW);

If you never mix analogWrite and digitalRead/Write on the same pin, you can
boost performance on PWM-enabled pins (making them equal with
non-PWM-enabled pins) by adding a second line above the first:

    #define DIGITALIO_NO_MIX_ANALOGWRITE
    #include "digitalIOPerformance.h

Digital read & write performance will be higher, and the compiled
sketch size will be smaller.

... if you still want to mix analogWrite() and
digitalRead/digitalWrite() on the same pin, but also use
DIGITALIO_NO_MIX_ANALOGWRITE, then you can use the noAnalogWrite()
function in between to turn off the PWM output:

    analogWrite(MY_PIN, 120);
    // ... do some stuff
    noAnalogWrite(MY_PIN);
    digitalWrite(MY_PIN, LOW);

## Option: Last shreds of performance

Under some circumstances the Arduino digital functions have to protect
against [interrupts](http://www.uchobby.com/index.php/2007/11/24/arduino-interrupts/) occuring while an I/O read or write is in progress. This can cause things to get out of sync if an I/O operation takes more than a single instruction to process ("interrupt unsafe".)

When using digitalIOPerformance, most writes only take one instruction anyway, so they are naturally "interrupt safe". However some cases have to be made "interrupt safe":

* Writing to certain pins on Arduino Mega models.

* Setting pinMode() to INPUT_PULLUP.

By default, "digitalIOPerformance" makes these cases interrupt safe,
in order to keep them as safe as the built-in Arduino
versions. However if your Arduino sketch doesn't use interrupts, or
you don't care about interrupt safety for digital I/O, then you can
add another line to get faster digital I/O on them:

    #define DIGITALIO_NO_INTERRUPT_SAFETY
    #include "digitalIOPerformance.h

Only do this one if you're very sure you don't need interrupt safe
digital reads & writes. If you're already using
DIGITALIO_NO_MIX_ANALOGWRITE then the only real improvement is on an
Arduino Mega, and even then only on some of the pins.

## Option: Disable automatic performance boost

If you don't want the library to automatically replace your
digitalRead/digitalWrite/pinMode function calls, you can do that as
well:

    #define DIGITALIO_MANUAL
    #include "digitalIOPerformance.h

You can still use the functions in the library if you call them by
their original names (given below.)

# Functions Defined

These functions are defined by the library:

## digitalWriteSafe / digitalReadSafe / pinModeSafe

These versions of digitalWrite/digitalRead & pinMode run faster
than the built-in Arduino versions, if the pin number is known at
compile time.

They are also just as safe as the built-in Arduino versions - they're
interrupt safe, and they disable any previous analogWrite() calls.

When you include the library, these functions automatically replace
the built-in digitalWrite & pinMode functions. If you don't want this
to happen, define DIGITALIO_MANUAL before including (as shown above.)


## digitalWriteFast / digitalReadFast / pinModeFast

These versions of digitalWrite/digitalRead & pinMode will usually
compile down to a single port register instruction (as fast as is
possible to be) if the pin number is known at compile time. If the pin
number is a variable then they fall through to the slower Arduino
version if the pin number is a variable.

You can have these functions automatically replace all Arduino
digitalWrite/digitalRead & pinMode functions if you include the
library thus:

    #define DIGITALIO_NO_INTERRUPT_SAFETY
    #define DIGITALIO_NO_MIX_ANALOGWRITE
    #include "digitalIOPerformance.h

## noAnalogWrite

Using digitalWriteFast() will not automatically turn off a
previous analogWrite() to that port, unlike Arduino's digitalWrite().

If you are mixing analogWrite() and digitalWriteFast() on a port, call
this function after immediately before calling digitalWriteFast(), if
you had previously called analogWrite().

The "safe" methods already call noAnalogWrite() any time you access a
PWM-capable pin, unless you've defined DIGITALIO_NO_MIX_ANALOGWRITE.


# Status

New, untested, hacky, work in progress. :)

Please raise an issue if this doesn't work with your Arduino install,
or doesn't seem to inline properly (ie massively bloated code size
instead of shrinking code size!)

Minimal testing done with Windows Arduino 1.0.3 (gcc 4.3) and my
Ubuntu Arduino 1.0.3 (gcc 4.7.) Should work with any Arduino version
above 1.0 (I think.)

# Known Shortcomings

* No ARM support, does nothing at all on the Arduino Due.

# Internal Workings

digitalIOPerformance.h is code generated automatically from an
existing Arduino installation by the script generateDigitalIOHeader.py.

You shouldn't need to run the code generation script unless you have a
newer/different Arduino version than the one it was last run against.

However, having code generation means it should be simple to update
against future new boards like the Leonardo (assuming the file
formats don't change much.)

# Thanks

Big thanks to the authors of digitalWriteFast - Paul Stoffregen, Bill
Westfield, an John Raines. I wrote this instead of updating
digitalWriteFast.h to support the Leonardo (code generation was more
appealing than handwritten bit fiddles!)

Also thanks to Alastair D'Silva who told me a while ago about the
trick of preprocessing pins_arduino.h to extract Arduino pin
information, he uses this in the performance-oriented AVR library
[MHVLib](http://www.makehackvoid.com/project/MHVLib).

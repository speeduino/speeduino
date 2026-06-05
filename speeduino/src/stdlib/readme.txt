This folder contains various C++ standard library polyfills

Since the AVR compiler doesn't ship with a standard library, we can:
a. Add a 3rd party implementation via a library
b. Replicate those portions we need.
Currently we have option b.

If we ever adopt option a., this folder can be removed.
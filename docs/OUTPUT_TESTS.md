# Output test mode

The four Hardware Testing dialogs replace the previous injector/ignition tester
in the standard firmware and `reference/speeduino.ini`. There is no board-ID
restriction or changed pin mapping. The firmware definition signature is `speeduino 202504-outputtest1`; firmware and INI must match.

## Build and setup

Use the normal PlatformIO environment for the board, for example:

```text
pio run -e megaatmega2560
pio run -e black_F407VE
pio run -e teensy41
```

Select the board normally in TunerStudio. Keep its existing pin mapping and
review configured outputs, idle algorithm and ignition polarity. Back up the tune
before testing output controls. No special Levin or STM32 tester target is
required. The normal STM32 USB/UART choice follows the build environment.

## Controls

Every dialog has explicit Enable and Disable controls. Enable requires a stopped
engine, inactive schedules, no trigger logging, a valid primary trigger input,
and at least five seconds since boot. Only one tester session can own the engine
outputs at a time. Configuration writes are rejected until Disable.

Stop ends the test and turns its owned outputs off, but retains test mode.
Disable returns control to the normal firmware, which may operate outputs again.
Closing a dialog sends Disable through `clickOnClose`. Trigger activity, loss of
live-data/status polling for two seconds, and precision-timer faults stop outputs
and disarm the session. After a fault, Disable must precede another Enable.
Power-up always starts with tests disabled.

### Injector and coil

Selections are All available outputs, individual 1-8, and Configured outputs only.
All uses the channels compiled into the firmware that are present in the board
mapping. An explicit unavailable selection is rejected. Configured resolves
scheduler callbacks, including paired/semi-sequential, staged, wasted COP and
single-coil arrangements. Unknown routing is rejected rather than guessed from
cylinder count. Counts are per selected output; All drives them together, with
normal GPIO/SPI execution skew, rather than simulating firing order.

Defaults for both dialogs are All, 100 ms period and 100 events. Injector open
time defaults to 1 ms; coil dwell defaults to 2.5 ms. Injector open range is
0.100-20 ms; coil dwell is 0.100-5 ms with a minimum 10 ms period. Period includes
open and closed time, must allow at least 0.100 ms off, and can extend to 500 ms.
Count is 1-65535 and period times count cannot exceed 120 seconds.

Open/dwell entry resolution is 0.001 ms, period entry resolution is 0.01 ms.
Actual timing is quantized by the board scheduler timer and includes ISR/driver
latency; displayed precision is not a guarantee of physical accuracy. Raw tests
add no injector dead time, fuel corrections or voltage-based dwell correction.
Coil polarity and direct GPIO versus MC33810 routing use the existing drivers.
Coil testing does not generate tachometer pulses. Continuous coil On is omitted.

Duration is `(1 + (count - 1) * period + open) / 1000` seconds, including the
initial 1 ms delay and excluding an extra closed interval after the last event.
RPM gauges are arithmetic estimates, not an engine simulator. The four-stroke
single-output estimate is `120000 / period_ms`; the aggregate estimate divides
by the configured cylinder count. Wasted spark's engine RPM is half the
four-stroke single-output estimate. Edits apply to the next test only.

The injector dialog includes a separate Fuel Pump box. Pump On does not start
injector pulses. Off, sequence completion, Stop, Disable, a fault or 120 seconds
continuous pump operation switches it off. Repeated On does not extend the limit.

### Auxiliary outputs

Pump, A/C compressor and A/C fan have On/Off. Fan, boost, VVT1/2, WMI and PWM
Idle1/2 also offer 50% duty. Multiple AUX outputs can run together, and Off affects
only its column. Each feature must be enabled in the tune; Idle2 additionally
requires dual-channel PWM idle. WMI and VVT2 are mutually exclusive. WMI drives
its PWM and enable pins together; the empty-tank input and indicator are not
actuated. Both pins are checked before either is driven.

Shared frequency is 1-200 Hz in 0.1 Hz increments. Enter a value, then press 50%
to apply it to pulsed outputs. AUX uses a 1 ms phase accumulator, so individual
edges are quantized and fractional periods alternate tick lengths. There is no
fixed AUX runtime limit; the communication and trigger interlocks remain.
Idle1/2 are independent electrical tests: On is HIGH and Off is LOW. They are
not stepper driver coils. Normal PWM cannot overwrite a claimed pin.

### Idle valve

The separate idle dialog uses the configured PWM or stepper algorithm. Disabled
or unsupported algorithms cannot start a test. Idle tests retain a 30 s limit.

Stepper Home moves towards the closed stop for 1-765 steps. Run first homes then
moves to a selected 0-255 position, below home count and within configured maximum
travel. Cycle alternates zero and the selected position, holding for one second
at each end. STEP time, cooling time and direction follow the tune. Zero STEP
time is rejected; zero cooling uses 1 ms. DIR has a full millisecond setup.
Position is estimated, not measured. An interrupted home/high STEP makes the
position unknown, causing normal control to home again after release.

PWM idle uses 100 Hz and 0-100% duty, with single or complementary dual outputs
and the configured direction. Home requests zero duty, Run holds duty, and Cycle
alternates duty and zero. Stop de-energizes both outputs. The feedback gauge is
gated to the idle session and shows estimated steps or duty. Homing shows zero
until the reference is established.

## Pin ownership and timing

Each selected pin is validated before activation. A conflict rejects the whole
selection, including All. Inputs, programmable outputs, enabled AUX/idle,
reserved pins and cross-assignments are checked. An unused injector/coil GPIO
can be reused by AUX, but cannot simultaneously be selected as a raw test output.
MC33810 outputs are logical driver channels; their SPI pins cannot be claimed
by AUX. These checks are test admission checks, not global tune validation.

Fan, boost, VVT and PWM idle protect their physical writes through the shared
`BenchOutputPin` adapter. PWM phase tracking and compare scheduling continue
while a pin is claimed; the controllers do not look up global pin numbers in
their interrupt handlers to decide whether to run. Releasing ownership permits
normal pin writes again without reinitialising the output objects.

The tester borrows the existing fuel-1 compare channel only while normal engine
control is suspended. Long waits are split at intermediate compare deadlines
without adding output edges. The hardware timer's counter is not reset. Normal
scheduler callbacks are preserved. A late ISR (>100 us) or stalled timer aborts;
a separate millisecond watchdog remains active while main-loop control is paused.
Critical sections preserve AVR/ARM interrupt state and native-test locking.

Only valid, configured primary/secondary/tertiary trigger interrupts are
replaced with abort handlers. Disable resets decoder state and restores their
callbacks and edges. There are no fixed STM32 trigger pins or extra TIM7 use.
The old test activation/pulsing commands are rejected. Legacy Disable remains
accepted, including when RPM is nonzero.

## RAM protocol and persistent storage

The original 15 persistent pages and EEPROM offsets are unchanged. Page 16 is a
24-byte RAM-only settings page with no burn command, noMsqSave and
controllerPriority. Old persistent hardware-test duration fields remain reserved
in their original locations; they no longer control testing. Settings reset at
reboot. The tester also uses runtime state in RAM; 24 bytes is the settings page
size, not the total feature RAM cost.

CRC-framed `N` commands, with little-endian U16 settings:

| Opcode | Operation |
|---|---|
| 00 | Disable/release |
| 01 | Start injector sequence |
| 02 | Status and renew 2 s lease |
| 03 offset:u16 count:u16 | Read settings |
| 04 offset:u16 count:u16 data | Stage settings |
| 05 | Settings CRC32, big-endian response |
| 06 target:u8 mode:u8 | AUX Off=0, On=1, 50%=2 |
| 07 | Start coil sequence |
| 08 state:u8 | Injector-session fuel pump |
| 09 mode:u8 | Idle Home=0, Run=1, Cycle=2 |
| 0A kind:u8 | Enable injector=1, coil=2, AUX=3, idle=4 |
| 0B | Stop while retaining enabled session |

Settings offsets: 0 injector selection, 2 open microseconds, 4 period in 10 us
units, 6 count; 8 coil selection, 10 dwell microseconds, 12 period in 10 us units,
14 count; 16 home steps, 18 run steps, 20 idle duty percent, 22 AUX frequency in
0.1 Hz units. Selection 0 means All, 1-8 individual, 9 configured.

AUX IDs: 0 pump, 1 fan, 2 boost, 3 VVT1, 4 VVT2, 5 WMI, 6 A/C compressor,
7 A/C fan, 8 Idle1, 9 Idle2. N02 returns result, version (8), state, reason,
channel:u16, on-us:u16, period-10us:u16, count:u16, completed:u16, owned.
States: idle=0, waiting=1, on=2, complete=3, aborted=4. Fault reasons: link=1,
trigger=2, late ISR=3, stalled timer=4, pump timeout=5; zero means no fault.

Telemetry byte39: bits0:1 kind minus one, bit2 owned, bit3 running, bit4 complete,
bit5 fault, bit6 enabled. Byte38 holds idle test feedback. Packet size is unchanged.

## Verification

```text
python tools/test_injector_bench.py
python tools/test_bench_ini.py
pio test -e native_code_coverage -f test_commandcontrol -f test_schedules -f test_loggers
```

Host tests compile the actual controller with simulated registers and outputs.
They cover commands, defaults, staging, single/All/configured selection, pin
conflicts, timer wrap/splitting, trigger restoration, Stop/Disable, timeouts, pump,
concurrent AUX, WMI and idle. Additional variants cover 4/5, 6/3, 8/1 and 8/8
channel counts and logical SPI drivers. INI tests check page commands, fields,
menu ordering, feature gates and gauge labels. These checks do not actuate an ECU.

Before use with real loads, verify physical waveforms and pulse counts on each
MCU/driver family, including minimum dwell/open time, maximum period, All-output
skew, both ignition polarities and heavy communications. Check trigger abort,
USB/serial loss, window close, explicit Stop/Disable and reconnection. Verify
stepper direction, home/run/cycle, interruption and return to normal control,
plus single/complementary PWM idle and concurrent AUX at frequency boundaries.

Use suitable dummy loads and a scope first. Injector flow tests belong on a
separate bench; coils need appropriate dwell and grounded spark loads. Hardware
testing on one board does not validate every MCU, driver or wiring arrangement.

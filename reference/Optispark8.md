# GM Gen II LT1 Optispark: low-resolution track only

This decoder uses the eight-window optical track on the primary trigger input.
The 360-slot track is not needed. It is separate from the existing Nissan 360
V8 Optispark path, which measures the low-resolution windows using high-resolution teeth.

## Configuration

- Use the matching firmware and INI; select **GM LT1 Optispark (8-slot only)**.
- Configure eight cylinders, four-stroke operation and the correct firing order/output wiring.
- Connect Low-res to the primary trigger input. No secondary trigger is required.
- Both edges are always captured. **Trigger edge = RISING** expects evenly spaced
  rising edges and the high widths below. Choose **FALLING** if the input circuitry
  inverts that waveform. Here this setting selects polarity, not single-edge sampling.
- Primary tooth count and trigger speed are not used; the input always runs at cam speed.
- The decoder checks every edge and the sync marker every cycle. The generic
  trigger-filter and optional re-sync settings are not used. Per-tooth ignition
  adjustment is unsupported and is disabled when initializing this decoder.
- Set the trigger angle using an actual cylinder #1 reference. Selecting the pattern
  does not install an LT1 base tune or a known TDC offset. Do not copy a numeric
  trigger offset from rusEFI or another ECU: angle conventions can differ.

The reference waveform comes from [Ardu-Stim revision 51f624a9](https://github.com/speeduino/Ardu-Stim/blob/51f624a9ae369035211952862befe099c2b93a58/ardustim/ardustim/wheel_defs.h),
`optispark_lt1`, low-resolution bit. Angles are crank degrees over a 720-degree cycle:

| Rising | Falling | High width |
| --- | --- | --- |
| 86 | 100 | 14 |
| 176 | 180 | 4 |
| 266 | 290 | 24 |
| 356 | 360 | 4 |
| 446 | 480 | 34 |
| 536 | 540 | 4 |
| 626 | 670 | 44 |
| 716 | 720 | 4 |

The table origin is **not verified cylinder #1 compression TDC**. Synchronization
is acquired on the falling edge at reference angle 100, using consecutive interval
ratios 14/86, 86/4 and 4/46. The third ratio rejects the reversed reference pattern.
Once synchronized, every interval is compared with its expected angular length,
allowing one-third variation relative to the most recent 90-degree period. This
is a starting tolerance tested with synthetic signals, not a measured cranking envelope.

RPM and between-edge interpolation use the 90-degree rising-to-rising period,
so alternating window widths do not create alternating RPM readings. The decoder
reports 720-degree phase directly, independently of the hidden TrigSpeed setting.
A malformed edge or timeout clears phase; clean history is required to reacquire.

## Tests and engine validation

Automated tests feed independent absolute edge timestamps through the decoder.
They cover all 16 starting edges at 41/80/200/1000/6500/18000 RPM with both polarities,
reverse rotation, invalid equal-width signals, missing/extra pulses, duplicate
edges, stop/reset/restart, micros() wrap through zero, changing speed with jitter,
RPM scaling, trigger-angle offsets and 360/720-degree interpolation.

These are synthetic tests. Real engine validation must include:

1. Record the primary composite log during cranking and running; verify polarity,
   pulse widths/order, RPM and absence of sync loss. Keep the original ECU in
   control during initial signal observation where practical.
2. Verify the trigger offset and **compression** phase of cylinder #1. A timing
   light cannot distinguish compression from exhaust TDC (360 crank degrees apart).
3. With fixed ignition timing, check timing at cranking, idle and several running
   speeds. Speeduino uses its separate cranking advance below the cranking threshold;
   set that to the same angle for this test. Confirm cylinder trims/corrections.
4. Repeat acquisition/recovery during real compression-induced speed changes.

There are only 16 position updates per engine cycle; do not assume the cranking
or transient timing accuracy of the 360-slot track. No engine test is claimed.

## Integration

Trigger ID 29 occupies an unused value in the existing five-bit TrigPattern field.
No EEPROM/page layout or board pin mapping changes are needed. The definition is
board-independent and is also selectable with SMALL_FLASH_DECODER=29.

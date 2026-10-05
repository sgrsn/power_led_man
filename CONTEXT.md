# Power LED Demo

ATtiny85 drives an RGB power LED through three FETs, producing smooth light patterns.

## Language

**Channel**:
One of the three independently dimmable colour outputs: R, G, or B.
_Avoid_: pin, output, LED (when meaning a single colour)

**Pattern**:
A named behaviour that determines the brightness of every Channel as a continuous function of time. The four Patterns are: Breathing, Hue Cycle, Candle, RGB Chase.
_Avoid_: mode, effect, sequence

**Lighting**:
The span from turning the rocker switch on to turning it off. Exactly one **Pattern** runs for the whole Lighting, looping forever. Powering up with the switch already on also starts a Lighting.
_Avoid_: session, power cycle, run

**Pattern Rotation**:
The rule that each new **Lighting** runs the next **Pattern** in a fixed cyclic order, remembered across power loss.
_Avoid_: mode switch

**Brightness Cap**:
A single global upper limit on every **Channel**'s brightness, protecting the power LED from overheating.
_Avoid_: max duty, limit

**Keep-Alive**:
Short, periodic current pulses through a dummy load that stop the battery's automatic shut-off. Runs all the time, whether or not anything is lit.
_Avoid_: dummy load (that is the resistor, not the behaviour), heartbeat

**Smooth**:
Brightness changes without any visible step or jump, as perceived by the human eye.
_Avoid_: soft, gradual

## Relationships

- The RGB power LED has exactly three **Channels**: R, G, B
- A **Pattern** sets all three **Channels** at once
- Every **Pattern** must be **Smooth** within itself
- A **Lighting** starts from all-off and fades in, and fades out when the switch is turned off, so switching is also **Smooth**
- One **Lighting** runs exactly one **Pattern**; **Pattern Rotation** picks it
- **Keep-Alive** is independent of **Lighting**: it matters most while nothing is lit
- No **Channel** ever exceeds the **Brightness Cap**

## Flagged ambiguities

- "滑らかに切り替わる" was resolved to mean changes *within* a **Pattern** are **Smooth**. Switching between Patterns happens only across **Lightings**, so no in-run transition exists.

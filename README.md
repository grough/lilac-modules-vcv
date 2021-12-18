# Lilac Modules for VCV Rack

## Accumulator

The Accumulator module calculates a sum of voltage over time, with support for
mono and polyphonic signals. The module is split vertically into two identical,
independent sections.

A signal of N volts connected to the _RATE_ input will accumulate at N volts per
second on the _SUM_ output. For example, a constant input of 1V will accumulate
to 10V over 10 seconds. A polyphonic cable connected to the _RATE_ input will
produce a signal with corresponding polyphony on the _SUM_ output.

A trigger on the _RESET_ input will reset the _SUM_ output to zero. A trigger
sent to the _RESET_ input over a polyphonic cable will clear the sum stored on
the corresponding polyphony channel. To completely reset the module's internal
state, select _Initialize_ from the module's menu.

The module's internal state is saved with your patch file, meaning that
accumulated values will be retained across Rack sessions.

## Comparator

The Comparator module compares two input voltages _A_ and _B_, with support for
mono and polyphonic signals.

Voltage _A_ can be set using the knob control, or by connecting a cable to the
nearby input port. Connecting a cable to the _A_ input port will disable the
knob control. Voltage _B_ can only be set using its input port.

The tolerance for what is considered equal on the _A = B_ output is determined
by the _A = B tolerance_ control in the module's menu. The minimum (and
default) tolerance is a very small non-zero number that reads as _±0.000V_ on
the slider. The tolerance can be increased to a maximum of ±1V by clicking and
dragging the slider. Polyphonic cables connected to the _A_ and/or _B_ input
ports will produce a signal with corresponding polyphony on all output ports.

## Looper

See separate [Lilac Loop](https://github.com/grough/lilac-loop-vcv) plugin.

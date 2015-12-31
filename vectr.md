## Color Modes

Color modes make use of up to 3 color palettes. Each palette has a matching number of colors. Unused colors are assumed to be blank.

* **Static** - Only use one color palette
* **FrameDrag2** - Interpolate 2 color palettes based on speed
* **FrameDrag3** - Interpolate 3 color palettes based on speed
* **TiltShift2** - Interpolate 2 color palettes based on pitch
* **TiltShift3** - Interpolate 3 color palettes based on pitch

## Patterns

Only strobe, tracer, and edge are used as bases for this firmware.

Ideas:
- Can flip patterns?

* (12) Static Patterns
    * Ribbon
    * Strobe
    * Dops
    * Hyper
    * Strobie
    * Faint
    * Blaster
    * Tracer
    * Vexing
    * Sword
    * Razor
    * Cyclops
* Dynamic Patterns
    * (11) Strobes (st/bt/lt)
        * 0/25/0 to 25/0/0              (Blank -> Ribbon)
        * 0/25/0 to 3/22/0              (Blank -> Strobie)
        * 0/50/0 to 25/25/0             (Blank -> Hyper)
        * 0/0/75 to 5/0/60              (Blank -> Blast)
        * 5/0/60 to 25/0/0              (Blast -> Ribbon)
        * 5/0/60 to 3/22/0              (Blast -> Strobie)
        * 5/0/60 to 5/10/60             (Blast -> Autoblast)
        * 5/0/35 to 5/0/85              (Blast speed)
        * 1.5/11/0 to 1.5/48.5/0        (Dops distance increases times 4)
        * 25/0/0 to 25/50/0             (Hyper distance increases from 0 to double)
        * 5/10/0 to 25/25/0             (Strobe -> Hyper)
    * (5) Single tracer (cst/cbt/tst/tbt)
        * 0/0/25 to 3/0/22              (Solid -> Tracer)
        * 3/11/0 to 3/0/22              (Strobie -> Tracer even gap)
        * 25/0/0 to 3/0/22              (Ribbon -> Tracer)
        * 3/9.5/0 to 3/0/47             (Tracer increases from half to double)
    * (5) Multi tracer (cst/cbt/tst/tbt)
        * 3/22/1.5/11 to 3/22/0/12.5    Perplexing
        * 3/22/12.5/0 to 3/22/0/12.5    (Vex from ribbon -> blank)
        * 3/22/1.5/0 to 3/22/1.5/48.5   (Vex gap increases)
        * 0/0/1.5/11 to 3/22/1.5/11     (No strobie -> strobie)
        * 25/0/1.5/11 to 3/22/1.5/11    (Ribbon -> strobie)
    * (5) Edge (cst/cbt/est/ebt)
        * 5/10/0/0 to 1.5/0/4/65        (Strobe -> Razor)
        * 1.5/13.5/0/0 to 1.5/0/4/65    (Dops -> Razor)
        * 1.5/0/4/65 to 4.5/0/12/45     (Razor -> Sword)
        * 3/22/3/0 to 2/7/110           (Strobie -> Razor)
        * 12/0/2/0 to 1.5/0/4/40        (Cyclops -> Razor)

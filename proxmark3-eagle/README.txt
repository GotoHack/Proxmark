
These are the original schematics and layouts for the proxmark3. These
files were used to generate any Gerbers or other output. They were drawn
in Cadsoft's EAGLE; a restricted version is available for free download at

   http://www.cadsoft.de/ 

and should be capable of viewing this design.


lf-antenna.brd is a printed circuit board antenna designed for ~125 kHz;
it's just an air-core spiral. The inductance ends up too low to resonate
with the 1000 pF on the proxmark3. You could either increase that 1000
pF, or put ~1220 pF in parallel across the coil. I did the latter,
using the provided pads.

Jonathan Westhues
Feb 12 2009
Bellevue WA


Command Protocol
================

| Command        | Code   | Arg 0     | Arg 1      | Arg 2      | Function                         |
|----------------|--------|-----------|------------|------------|----------------------------------|
|  Write         | 100    | Byte      | Value      |  -         | Writes value to byte             |
|  Handshake     | 200    | Version   | Check Val  |  Check Val | Initiates connection             |
|  Disconnect    | 210    | -         | -          |  -         | Disables connection              |
|  View Mode     | 220    | -         | -          |  -         | Light will display mode preview  |
|  View Color    | 230    | Color Set | Color Slot |  -         | Light will display color preview |

1 - App sends handshake to light (200, *version*, *value*, *value*)
2 - Light responds with handshake (200, *version*, *value*, *value*)
3 - Light enters "View Mode" state disabling buttons
4 - App sends write commands to light for mode previewing
5 - App sends disconnect to light (210, *any*, *any*, *any*)
6 - Light returns to default "Play" state and buttons are re-enabled


Mode Structure
==============

* Only for Vectr modes
+ Only for Primer modes

| Byte   | Function                                                                         |
|--------|----------------------------------------------------------------------------------|
|   0    |  Mode type (0 - Vectr, 1 - Primer)                                               |
|   1    |  Pattern 0                                                                       |
|   2    |  +Pattern 1                                                                      |
|   3    |  ArgSet 0 Arg 0                                                                  |
|   4    |  ArgSet 0 Arg 1                                                                  |
|   5    |  ArgSet 0 Arg 2                                                                  |
|   6    |  ArgSet 0 Arg 3                                                                  |
|   7    |  +Arg Set 1 Arg 0                                                                |
|   8    |  +Arg Set 1 Arg 1                                                                |
|   9    |  +Arg Set 1 Arg 2                                                                |
|  10    |  +Arg Set 1 Arg 3                                                                |
|  11    |  Timing Set 0 Timing 0                                                           |
|  12    |  Timing Set 0 Timing 1                                                           |
|  13    |  Timing Set 0 Timing 2                                                           |
|  14    |  Timing Set 0 Timing 3                                                           |
|  15    |  Timing Set 0 Timing 4                                                           |
|  16    |  Timing Set 0 Timing 5                                                           |
|  17    |  Timing Set 0 Timing 6                                                           |
|  18    |  Timing Set 0 Timing 7                                                           |
|  19    |  Timing Set 1 Timing 0                                                           |
|  20    |  Timing Set 1 Timing 1                                                           |
|  21    |  Timing Set 1 Timing 2                                                           |
|  22    |  Timing Set 1 Timing 3                                                           |
|  23    |  Timing Set 1 Timing 4                                                           |
|  24    |  Timing Set 1 Timing 5                                                           |
|  25    |  Timing Set 1 Timing 6                                                           |
|  26    |  Timing Set 1 Timing 7                                                           |
|  27    |  *Timing Set 2 Timing 0                                                          |
|  28    |  *Timing Set 2 Timing 1                                                          |
|  29    |  *Timing Set 2 Timing 2                                                          |
|  30    |  *Timing Set 2 Timing 3                                                          |
|  31    |  *Timing Set 2 Timing 4                                                          |
|  32    |  *Timing Set 2 Timing 5                                                          |
|  33    |  *Timing Set 2 Timing 6                                                          |
|  34    |  *Timing Set 2 Timing 7                                                          |
|  35    |  Number of Colors in Set 0                                                       |
|  36    |  Number of Colors in Set 1                                                       |
|  37    |  *Number of Colors in Set 2                                                      |
|  38    |  Color Set 0 Color 0 Red                                                         |
|  39    |  Color Set 0 Color 0 Green                                                       |
|  40    |  Color Set 0 Color 0 Blue                                                        |
|  41    |  Color Set 0 Color 1 Red                                                         |
|  42    |  Color Set 0 Color 1 Green                                                       |
|  43    |  Color Set 0 Color 1 Blue                                                        |
|  44    |  Color Set 0 Color 2 Red                                                         |
|  45    |  Color Set 0 Color 2 Green                                                       |
|  46    |  Color Set 0 Color 2 Blue                                                        |
|  47    |  Color Set 0 Color 3 Red                                                         |
|  48    |  Color Set 0 Color 3 Green                                                       |
|  49    |  Color Set 0 Color 3 Blue                                                        |
|  50    |  Color Set 0 Color 4 Red                                                         |
|  51    |  Color Set 0 Color 4 Green                                                       |
|  52    |  Color Set 0 Color 4 Blue                                                        |
|  53    |  Color Set 0 Color 5 Red                                                         |
|  54    |  Color Set 0 Color 5 Green                                                       |
|  55    |  Color Set 0 Color 5 Blue                                                        |
|  56    |  Color Set 0 Color 6 Red                                                         |
|  57    |  Color Set 0 Color 6 Green                                                       |
|  58    |  Color Set 0 Color 6 Blue                                                        |
|  59    |  Color Set 0 Color 7 Red                                                         |
|  60    |  Color Set 0 Color 7 Green                                                       |
|  61    |  Color Set 0 Color 7 Blue                                                        |
|  62    |  Color Set 0 Color 8 Red                                                         |
|  63    |  Color Set 0 Color 8 Green                                                       |
|  64    |  Color Set 0 Color 8 Blue                                                        |
|  65    |  Color Set 1 Color 0 Red                                                         |
|  66    |  Color Set 1 Color 0 Green                                                       |
|  67    |  Color Set 1 Color 0 Blue                                                        |
|  68    |  Color Set 1 Color 1 Red                                                         |
|  69    |  Color Set 1 Color 1 Green                                                       |
|  70    |  Color Set 1 Color 1 Blue                                                        |
|  71    |  Color Set 1 Color 2 Red                                                         |
|  72    |  Color Set 1 Color 2 Green                                                       |
|  73    |  Color Set 1 Color 2 Blue                                                        |
|  74    |  Color Set 1 Color 3 Red                                                         |
|  75    |  Color Set 1 Color 3 Green                                                       |
|  76    |  Color Set 1 Color 3 Blue                                                        |
|  77    |  Color Set 1 Color 4 Red                                                         |
|  78    |  Color Set 1 Color 4 Green                                                       |
|  79    |  Color Set 1 Color 4 Blue                                                        |
|  80    |  Color Set 1 Color 5 Red                                                         |
|  81    |  Color Set 1 Color 5 Green                                                       |
|  82    |  Color Set 1 Color 5 Blue                                                        |
|  83    |  Color Set 1 Color 6 Red                                                         |
|  84    |  Color Set 1 Color 6 Green                                                       |
|  85    |  Color Set 1 Color 6 Blue                                                        |
|  86    |  Color Set 1 Color 7 Red                                                         |
|  87    |  Color Set 1 Color 7 Green                                                       |
|  88    |  Color Set 1 Color 7 Blue                                                        |
|  89    |  Color Set 1 Color 8 Red                                                         |
|  90    |  Color Set 1 Color 8 Green                                                       |
|  91    |  Color Set 1 Color 8 Blue                                                        |
|  92    |  Color Set 2 Color 0 Red                                                         |
|  93    |  Color Set 2 Color 0 Green                                                       |
|  94    |  Color Set 2 Color 0 Blue                                                        |
|  95    |  Color Set 2 Color 1 Red                                                         |
|  96    |  Color Set 2 Color 1 Green                                                       |
|  97    |  Color Set 2 Color 1 Blue                                                        |
|  98    |  Color Set 2 Color 2 Red                                                         |
|  99    |  Color Set 2 Color 2 Green                                                       |
| 100    |  Color Set 2 Color 2 Blue                                                        |
| 101    |  Color Set 2 Color 3 Red                                                         |
| 102    |  Color Set 2 Color 3 Green                                                       |
| 103    |  Color Set 2 Color 3 Blue                                                        |
| 104    |  Color Set 2 Color 4 Red                                                         |
| 105    |  Color Set 2 Color 4 Green                                                       |
| 106    |  Color Set 2 Color 4 Blue                                                        |
| 107    |  Color Set 2 Color 5 Red                                                         |
| 108    |  Color Set 2 Color 5 Green                                                       |
| 109    |  Color Set 2 Color 5 Blue                                                        |
| 110    |  Color Set 2 Color 6 Red                                                         |
| 111    |  Color Set 2 Color 6 Green                                                       |
| 112    |  Color Set 2 Color 6 Blue                                                        |
| 113    |  Color Set 2 Color 7 Red                                                         |
| 114    |  Color Set 2 Color 7 Green                                                       |
| 115    |  Color Set 2 Color 7 Blue                                                        |
| 116    |  Color Set 2 Color 8 Red                                                         |
| 117    |  Color Set 2 Color 8 Green                                                       |
| 118    |  Color Set 2 Color 8 Blue                                                        |
| 119    |  Pattern Thresh 0 to 1 Start (Vectr) / Trigger Low (Primer)                      |
| 120    |  Pattern Thresh 0 to 1 End (Vectr) / Trigger High (Primer)                       |
| 121    |  *Pattern Thresh 1 to 2 Start (Vectr)                                            |
| 122    |  *Pattern Thresh 1 to 2 End (Vectr)                                              |
| 123    |  *Color Thresh 0 to 1 Start (Vectr)                                              |
| 124    |  *Color Thresh 0 to 1 End (Vectr)                                                |
| 125    |  *Color Thresh 1 to 2 Start (Vectr)                                              |
| 126    |  *Color Thresh 1 to 2 End (Vectr)                                                |
| 127    |  +Primer Trigger Type (0 - off, 1 - velocity, 2 - tilt, 3 - roll, 4 - flip)      |

# ADCconverter_lib
 Library for converting ADC data to real values.
 
Just add the library to your project:
 
```C
#include "ADCconverter.h"
#include "ADCtables.h"
```
 
And get a ready converted result from the ADC readings:
 
```C
float result = ADC_converter(ADC_value, YOUR_ADC_TABLE, TABLE_SIZE(YOUR_ADC_TABLE));
```

The library works better when combined with the RingBuffer noise-removing library.
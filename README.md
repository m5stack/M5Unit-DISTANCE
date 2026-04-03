# M5Unit - DISTANCE

## Overview

Library for DISTANCE using [M5UnitUnified](https://github.com/m5stack/M5UnitUnified).  
M5UnitUnified is a library for unified handling of various M5 units products.

### SKU:U098-B1

The Unit Ultrasonic-I2C is an ultrasonic distance measurement sensor with an I2C communication interface. The hardware utilizes the RCWL-9620 ultrasonic distance measurement chip paired with a 16mm probe, capable of achieving precise distance measurement within the range of 2cm-450cm (accuracy up to ±2%). As an I2C slave device, the sensor can share bus resources with other I2C devices, making it very suitable for multi-sensor applications such as robot obstacle avoidance, fluid level detection, and other scenarios requiring distance measurements.


### SKU:U098-B2

Unit Ultrasonic-IO is a GPIO interface ultrasonic distance measurement sensor. The hardware uses the RCWL-9620 ultrasonic distance measurement single-chip combined with a 16 mm probe, capable of achieving precise distance measurement within the range of 2 cm-450 cm (accuracy up to ±2%). After the controller sends a trigger signal, the sensor outputs a pulse duration corresponding to the sound wave reflection time, and the distance can be calculated from this data. The IO control method gives this sensor an extremely fast response speed, making it very suitable for applications such as robot obstacle avoidance and liquid level detection.


## Future Plans
- Unit Ultrasonic(SKU:U098) will be supported soon.


## Related Link
See also examples using conventional methods here.

- [Unit Ultrasonic-I2C & Datasheet](https://docs.m5stack.com/en/unit/ULTRASONIC%20I2C)
- [Unit Ultrasonic-IO & Datasheet](https://docs.m5stack.com/en/unit/UNIT%20SONIC%20IO)

### Required Libraries:
- [M5UnitUnified](https://github.com/m5stack/M5UnitUnified)
- [M5Utility](https://github.com/m5stack/M5Utility)
- [M5HAL](https://github.com/m5stack/M5HAL)

## License

- [M5Unit-DISTANCE - MIT](LICENSE)


### Support via [PbHub](https://docs.m5stack.com/en/unit/pbhub_1.1)

|Unit|Support|
|---|---|
|UnitUltraSonicIO|NG|

See also [M5Unit-HUB](https://github.com/m5stack/M5Unit-HUB)


## Examples
See also [examples/UnitUnified](examples/UnitUnified)

### For ArduinoIDE settings
You must choose a define symbol for the connection method you will use.
(Rewrite source or specify with compile options)

- PlotToSerial
```cpp
// *********************************************************************
// Choose connection
// *********************************************************************
#if !defined(CONNECT_VIA_I2C) && !defined(CONNECT_VIA_GPIO)
// UnitUltraSonicI2C (SKU:U098-B1)
// #define CONNECT_VIA_I2C
// UnitUltraSonicIO  (SKU:U098-B2)
// #define CONNECT_VIA_GPIO
#endif
```

### Doxygen document
[GitHub Pages](https://m5stack.github.io/M5Unit-DISTANCE/)

If you want to generate documents on your local machine, execute the following command

```
bash docs/doxy.sh
```

It will output it under docs/html  
If you want to output Git commit hashes to html, do it for the git cloned folder.

#### Required
- [Doxygen](https://www.doxygen.nl/)
- [pcregrep](https://formulae.brew.sh/formula/pcre2)
- [Git](https://git-scm.com/) (Output commit hash to html)



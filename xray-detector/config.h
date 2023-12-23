#ifndef CONFIG_H_
#define CONFIG_H_

#define SYSTEM_SERIAL_ENABLED true
#define SYSTEM_SERIAL_FREQ    9600

#define POWERSAVE_LIGHTSLEEP_DISABLE_IO true

// simulate encoder wia 3-buttons
#define USERINPUT_ENCODER_SIMUL false

// display type
#define DISPLAY_TYOE_ILI9341_ADAFRUIT false
#define DISPLAY_TYPE_ILI9341_CUSTOM true
#define DISPLAY_TYPE_ST7335_ADAFRUIT false
#define DISPLAY_TYPE_ST7335_CUSTOM false

// Enable hign voltage generator
#define UV_HIGH_VOLTAGE_OUTPUT_ENABLED true

// init fake accumulated data buffer on startup
#define EEPROM_CONTROL_INIT_FAKE_BUFFER false

#endif /* CONFIG_H_ */

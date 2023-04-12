#ifndef CONFIG_H_
#define CONFIG_H_

// simulate encoder wia 3-buttons
#define USERINPUT_ENCODER_SIMUL true

// display type
#define DISPLAY_TYOE_SIMUL_ADAFRUIT false
#define DISPLAY_TYPE_SIMIL_CUSTOM true
#define DISPLAY_TYPE_ST7335_ADAFRUIT false
#define DISPLAY_TYPE_ST7335_CUSTOM false

// Enable hign voltage generator
#define UV_HIGH_VOLTAGE_OUTPUT_ENABLED false

// init fake accumulated data buffer on startup
#define EEPROM_CONTROL_INIT_FAKE_BUFFER true

#endif /* CONFIG_H_ */

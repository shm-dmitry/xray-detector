#ifndef SVF_CONTROL_VIBRO_MODELS_H_
#define SVF_CONTROL_VIBRO_MODELS_H_

const uint16_t SVF_CONTROL_VIBRO_1[] PROGMEM = {1, 1, 20};
const uint16_t SVF_CONTROL_VIBRO_2[] PROGMEM = {5, 1, 100, 0, 100, 1, 100, 0, 100, 1, 100};
const uint16_t SVF_CONTROL_VIBRO_3[] PROGMEM = {5, 1, 1000, 0, 1000, 1, 1000, 0, 1000, 1, 1000};
const uint16_t SVF_CONTROL_VIBRO_4[] PROGMEM = {1, 1, 10000};

const uint16_t* const SVF_CONTROL_MODELS_VIBRO[] = {
  SVF_CONTROL_VIBRO_1,
  SVF_CONTROL_VIBRO_2,
  SVF_CONTROL_VIBRO_3,
  SVF_CONTROL_VIBRO_4
};

#endif /* NVF_CONTROL_VIBRO_MODELS_ */

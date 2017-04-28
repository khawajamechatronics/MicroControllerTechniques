#define POT BIT5
#define LDR BIT4
#define ANALOG_IN (POT | LDR)

#define LEDR BIT0
#define LEDG BIT1
#define LEDB BIT2
#define LEDS (LEDR | LEDG | LEDB)

#define P2_OUT 0x7F
#define P2_IN 0x80

#define ADC_VALUES 4

enum Color {
  NONE = 0,
  WHITE = 1,
  BLACK = 2,
  RED = 3,
  GREEN = 4,
  BLUE = 5
};

static const char* COLOR_NAMES[] = {
  "none",
  "white",
  "black",
  "red",
  "green",
  "blue"
};

static const uint16_t ADC_CHANNEL[ADC_VALUES] = {
  INCH_4, INCH_4, INCH_4, INCH_5
};

static const uint8_t ADC_LEDS[ADC_VALUES] = {
  LEDR, LEDG, LEDB
};

void setup(void);
void loop(void);

__inline void process_analog_value(uint8_t index, uint16_t value);
__inline void adc_convert(uint8_t index);

__inline void set_shift_register_leds(uint8_t state);
__inline void shift_register_clock(void);

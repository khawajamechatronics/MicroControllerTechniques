#define POT BIT5
#define LDR BIT4
#define ANALOG_IN (POT | LDR)

#define LEDR BIT0
#define LEDG BIT1
#define LEDB BIT2
#define LEDS (LEDR | LEDG | LEDB)

#define P2_OUT 0x7F
#define P2_IN 0x80

#define ADC_VALUES 2

enum Color {
  WHITE = 0,
  BLACK = 1,
  RED = 2,
  GREEN = 3,
  BLUE = 4
};

static const char* COLOR_NAMES[] = {
  "white",
  "black",
  "red",
  "green",
  "blue"
};

static const uint16_t ADC_CHANNEL[ADC_VALUES] = { INCH_4, INCH_5 };

void setup(void);
void loop(void);

__inline void update_analog_value(uint8_t index, uint16_t value);

__inline void set_leds(uint8_t state);
__inline void shift_register_clock(void);

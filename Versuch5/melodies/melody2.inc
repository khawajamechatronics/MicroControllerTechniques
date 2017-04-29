#include "../play.h"

#ifdef MLENGTH
#undef MLENGTH
#endif //MLENGTH

#define MLENGTH 3

const tone_t MDATA[MLENGTH] = {
  { 0x1000, 30 }, { 0x0100, 50 }, { 0x0300, 10 }
};
const melody_t MNAME = { MLENGTH, MDATA };

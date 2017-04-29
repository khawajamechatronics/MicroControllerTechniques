#include "../play.h"

#ifdef MLENGTH
#undef MLENGTH
#endif //MLENGTH

#define MLENGTH 3

const tone_t MDATA[MLENGTH] = {
  { 0x0500, 20 }, { 0x0200, 10 }, { 0x1000, 50 }
};
const melody_t MNAME = { MLENGTH, MDATA };

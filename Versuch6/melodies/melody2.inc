#include "../play.h"

#ifdef MLENGTH
#undef MLENGTH
#endif // MLENGTH

#ifdef MDATA
#undef MDATA
#endif // MDATA

#ifdef MNAME
#undef MNAME
#endif // MNAME

#define MNAME MELODY2
#define MDATA MELODY2_DATA
#define MLENGTH 3

const tone_t MDATA[MLENGTH] = {
  { 0x1000, 30 }, { 0x0100, 50 }, { 0x0300, 10 }
};
const melody_t MNAME = { MLENGTH, MDATA };

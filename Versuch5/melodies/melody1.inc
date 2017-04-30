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

#define MNAME MELODY1
#define MDATA MELODY1_DATA
#define MLENGTH 8

// Plays the musical scale with 1sec per note
const tone_t MDATA[MLENGTH] = {
  { 478, 16 }, // C4
  { 426, 16 }, // D4
  { 379, 16 }, // E4
  { 358, 16 }, // F4
  { 318, 16 }, // G4
  { 284, 16 }, // A4
  { 253, 16 }, // B4
  { 238, 16 } // C5
};
const melody_t MNAME = { MLENGTH, MDATA };

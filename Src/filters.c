
/* Includes ------------------------------------------------------------------*/
#include "filters.h"

/* Exported data -------------------------------------------------------------*/
const q15_t lp16000_50_1000[LP_FS16000_50_1000_LENGTH] = 
{
   -7,    -11,    -19,    -29,    -42,    -58,    -75,    -94, -113,
 -130,   -143,   -149,   -146,   -130,   -100,    -52,     16, 105,
  215,    347,    498,    665,    845,   1033,   1223,   1408, 1583,
 1741,   1876,   1982,   2055,   2093,   2093,   2055,   1982, 1876,
 1741,   1583,   1408,   1223,   1033,    845,    665,    498, 347,
  215,    105,     16,    -52,   -100,   -130,   -146,   -149, -143,
 -130,   -113,    -94,    -75,    -58,    -42,    -29,    -19, -11,
   -7
};

const q15_t hp16000_1000_50[HP_FS16000_1000_50_LENGTH] = 
{
   -6,     -5,     -6,     -6,     -5,     -1,      7,     19,     36,
   60,     89,    125,    166,    211,    258,    304,    346,    378,
  396,    394,    365,    302,    197,     41,   -178,   -477,   -882,
-1445,  -2273,  -3652,  -6635, -20753,  20753,   6635,   3652,   2273,
 1445,    882,    477,    178,    -41,   -197,   -302,   -365,   -394,
 -396,   -378,   -346,   -304,   -258,   -211,   -166,   -125,    -89,
  -60,    -36,    -19,     -7,      1,      5,      6,      6,      5,
    6
};

/* End of file ---------------------------------------------------------------*/

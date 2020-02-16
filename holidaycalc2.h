#ifndef macros
#define macros
#include "macros.h"
#endif

#ifndef holidaycalc2
#define holidaycalc2

struct mdtype {
         int m,d;
};

struct HolType {
         mdtype MLK,Pres,Easter,Mother,Memorial,Father,Labor,Columbus,Election,Thanksgiving;
         int year;
         bool valid;
};

RETURN HolType FUNCTION GetHolidays(int year);
#endif

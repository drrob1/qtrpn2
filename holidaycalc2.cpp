// #include "macros.h"  already in the other header files
#include "holidaycalc2.h"
#include "timlibc2.h"
/*
  REVISION HISTORY
  ----------------
  5 Apr 88 -- 1) Converted to M2 V3.03.
              2) Imported the REALLIB and CALLIB modules, and deleted their
                  code from here.  They were originally created here, but
                  it seems more appropriate to leave them in a central
                  module and import them.
  19 Mar 89 -- 1) Imported the newly created HPCALC to allow date arithmetic.
                  The rest of the code was modified to take advantage of this
                  new capability.
               2) Fixed a bug in the error reporting from the EASTER proc
                  so that a FOR index variable does not get assigned 0.
  30 Mar 89 -- 1) Fixed bug in the PR and HOL cmds that ignored 2 digit
                  years
               2) Added reminder to GREG help line to use quotes to force
                  the date to be taken as an ALLELSE TKNSTATE.
  26 Dec 90 -- 1) Utilized the GETTKNSTR procedure where appropriate.
               2) UL2 is used instead of UTILLIB.
               3) Added GETTKNEOL proc to deal with GREG & DOW cmds.
  25 Jul 93 -- 1) Dropping requirement of CALCCMD by passing cmdline thru
                   to GETRESULT.
               2) Allowed empty command line to quit after confirmation.
               3) Eliminated writing of trailing insignificant 0's from
                   arithmetic functions.
               4) Imported TKNRTNS instead of TOKENIZE.
               5) Eliminated need for GETTKNSTR by improving algorithm.
               6) Deleted GETTKNEOL proc as it is no longer used.  If
                   needed, it may be imported from TKNRTNS now.
  18 May 03 -- Conversion to Win32 using Stony Brook Modula-2 V 4
  25 Dec 14 -- Converted to a module to get holiday dates by the calculator, using HolMod written for the Cal program.
   1 Jan 15 -- Converting to cpp, and combining HolMod into this module.
  20 Jan 20 -- Started coding version 2 of this rtn, now called holidaycalc2.cpp.  It will return a holiday struct, not
                  rely on an extern variable to be needed.  I'm more comfortable with Go syntax.  And by extension, C++ also.
*/

//-------------------------------- SUBTDAYS --------------------------------

RETURN int FUNCTION SUBTDAYS(int C, int Y) IS
/*
Subtract Days.
Computes how many days to subtract from the holiday depending on the year.
Days to Subtract = C + [5/4 Y] - [3/4 (1 + [Y/100])  ]) MOD 7
*/
  return ((C + (5*Y/4) - 3*(1 + (Y/100)) / 4) MOD 7);
ENDFUNC;// SUBTDAYS

//-----------------------------------------------------------------------------------------
RETURN int FUNCTION CalcMLK(int year) {
/*
 Find the date of MLK day by finding which day back from Jan 21 is a Monday.
*/

  int J, day;

  day = 21;
  J = JULIAN(1,day,year);
  WHILE (J MOD 7) NE 1 DO
    day--;
    J = JULIAN(1,day,year);
  ENDWHILE;
  return day;
};// CalcMLK

//------------------------------------ easter ------------------------------
RETURN mdtype FUNCTION easter(int YEAR) {
/*
EASTER.
This routine computes the golden number for that year, then Easter Sunday is
the first Sunday following this date.  If the date is a Sunday, then Easter
is the following Sunday.
*/
int GOLDENNUM;
int JULDATE;
mdtype md;
MDYType mdy;

  IF (YEAR < 1900) OR (YEAR > 2500) THEN
    md.m = 0;
    md.d = 0;
  ELSE
    GOLDENNUM = (YEAR MOD 19) + 1;
    SWITCH GOLDENNUM DO
      case  1: // APR 14
        md.m = 4;
        md.d = 14;
        break;
      case  2: // APR 3
        md.m = 4;
        md.d = 3;
        break;
      case  3: // MAR 23
        md.m = 3;
        md.d = 23;
        break;
      case  4: // APR 11
        md.m = 4;
        md.d = 11;
        break;
      case  5: // MAR 31
        md.m = 3;
        md.d = 31;
        break;
      case  6: // APR 18
        md.m = 4;
        md.d = 18;
        break;
      case  7: // APR 8
        md.m = 4;
        md.d = 8;
        break;
      case  8: // MAR 28
        md.m = 3;
        md.d = 28;
        break;
      case  9: // APR 16
        md.m = 4;
        md.d = 16;
        break;
      case 10: // APR 5
        md.m = 4;
        md.d = 5;
        break;
      case 11: // MAR 25
        md.m = 3;
        md.d = 25;
        break;
      case 12: // APR 13
        md.m = 4;
        md.d = 13;
        break;
      case 13: // APR 2
        md.m = 4;
        md.d = 2;
        break;
      case 14: // MAR 22
        md.m = 3;
        md.d = 22;
        break;
      case 15: // APR 10
        md.m = 4;
        md.d = 10;
        break;
      case 16: // MAR 30
        md.m = 3;
        md.d = 30;
        break;
      case 17: // APR 17
        md.m = 4;
        md.d = 17;
        break;
      case 18: // APR 7
        md.m = 4;
        md.d = 7;
        break;
      case 19: // MAR 27
        md.m = 3;
        md.m = 27;
    ENDCASE;
  ENDIF;
/*
  Now find next Sunday.
*/
  JULDATE = JULIAN(md.m,md.d,YEAR);
  WHILE (JULDATE MOD 7) NE 0 DO
    JULDATE++;
  ENDWHILE;
  mdy = GREGORIAN(JULDATE);
  md.m = mdy.m;
  md.d = mdy.d;
  return md;
}; // EASTER

//*****************************************************************************************

RETURN HolType FUNCTION GetHolidays(int year) {

  HolType Holidays;  // var Holidays HolType  in Go syntax

  Holidays.year = year;

  Holidays.MLK.m = 1;
  Holidays.MLK.d = CalcMLK(year);

  Holidays.Pres.m = 2;
  Holidays.Pres.d = 21 - SUBTDAYS(2,Holidays.year-1);

  Holidays.Easter = easter(year);

  Holidays.Mother.m = 5;
  Holidays.Mother.d =  14 - SUBTDAYS(0,Holidays.year);

  Holidays.Memorial.m = 5;
  Holidays.Memorial.d = 31 - SUBTDAYS(2,Holidays.year);

  Holidays.Father.m = 6;
  Holidays.Father.d = 21 - SUBTDAYS(3,Holidays.year);

  Holidays.Labor.m = 9;
  Holidays.Labor.d = 7 - SUBTDAYS(3,Holidays.year);

  Holidays.Columbus.m = 10;
  Holidays.Columbus.d = 14 - SUBTDAYS(5,Holidays.year);

  Holidays.Election.m = 11;
  Holidays.Election.d = 8 - SUBTDAYS(1,Holidays.year);

  Holidays.Thanksgiving.m = 11;
  Holidays.Thanksgiving.d = 28 - SUBTDAYS(5,Holidays.year);
  
  return Holidays;
 }; // GetHolidays

// END holidaycalc.cpp

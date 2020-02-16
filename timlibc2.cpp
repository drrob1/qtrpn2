#include "timlibc2.h"

/*
  REVISION HISTORY
  ----------------
  14 Apr 92 -- Created JULIAN and GREGORIAN procs, which are accurate beyond 3/1/2100.
  25 Jul 93 -- Changed GREG2JUL and JUL2GREG limits to those imposed by the
                algorithm, ie, only years <2100 are now allowed.
  25 Jul 94 -- Changed limits of allowed years to 1700 from 1900.
  10 Nov 02 -- Converted to SBM2 Win v4.
  17 May 03 -- First Win32 version.
  26 May 03 -- Adjusted algorithm for Julian fcn so year has a pivot.
   6 Oct 13 -- Converted to gm2.
  19 Jan 14 -- Converted to Ada.
  17 Nov 14 -- Converted to C++.
   7 Dec 14 -- Removed ElapsedSec from the exported data type.  See timlibc.h.
  20 Jan 20 -- Started converstion to idiomatic C++.
   8 Feb 20 -- Bug in GREGORIAN slipped by here.  It was fixed in Go version July 2016.
*/

  const int ADIPM[12] =  {0,1,-1,0,0,1,1,2,3,3,4,4};

//  This is a typed constant that represents the difference btwn the last day
//  of the previous month and 30, assuming each month was 30 days long.
//  The variable name is an acronym of Accumulated Days In Previous Months.


// Declared in the header file, timlibc2.h
//  struct DateTimeType IS
//   time_t rawtime;
//   tm POINTER pTime;
//    int month,day,year,hours,minutes,seconds;
//    long int ElapsedSec;   Not here anymore.
//  END; // DateTimeType Record


//              ----------------------------------- TIME2MDY -------------------------
RETURN MDYType FUNCTION TIME2MDY() {
// System Time To Month, Day, and Year Conversion.

    MDYType mdy;
    DateTimeType DateTime;
    DateTime = GetDateTime();

    mdy.m = DateTime.month;
    mdy.d = DateTime.day;
    mdy.y = DateTime.year;
    return mdy;
};// TIME2MDY

// ---------------------------------------------------- GetDateTime -----------------------------------
RETURN DateTimeType FUNCTION GetDateTime() {
  DateTimeType DateTime;
  time_t raw;
  tm POINTER pTimeInfo;

  time(ADROF raw);
  pTimeInfo = localtime(ADROF raw);

  DateTime.rawtime = raw;
  DateTime.TimeInfo = *pTimeInfo;

  DateTime.month = pTimeInfo->tm_mon + 1;
  DateTime.day = pTimeInfo->tm_mday;
  DateTime.year = pTimeInfo->tm_year + 1900;
  DateTime.hours = pTimeInfo->tm_hour;
  DateTime.minutes = pTimeInfo->tm_min;
  DateTime.seconds = pTimeInfo->tm_sec;
  return DateTime;
};// GetDateTime

// ----------------------------------------- MDY2STR ---------------------------------------------------
RETURN string FUNCTION MDY2STR(int M, int D, int Y, string REF MDYSTR) {
// Month Day Year Cardinals To String.  By both returning a string as a param and as a function I have
// created a dual function, as I am allowed to not assign the return value and thereby use it as a
// procedure.

  const char DateSepChar = '/';
  string MSTR,DSTR,YSTR;
//  string IntermedStr;  old way of doing this
  MSTR = to_string(M);
  DSTR = to_string(D);
  YSTR = to_string(Y);
  string IntermedStr = MSTR + DateSepChar + DSTR + DateSepChar + YSTR;
  MDYSTR = IntermedStr;
  return IntermedStr;
}; // MDY2STR

// ------------------------------------------------ JULIAN ----------------------------------
RETURN int FUNCTION JULIAN(int M, int D, int Y) IS

  int M0,Y0;
  unsigned int Juldate;

  IF (Y < 30) THEN
    Y0 = Y + 2000 - 1;
  ELSIF (Y < 100) THEN
    Y0 = Y + 1900 - 1;
  ELSE
    Y0 = Y - 1;
  END; // if Y

  IF (M < 1) OR (M > 12) OR (D < 1) OR (D > 31) OR (Y < 1700) OR (Y > 2500) THEN
// Month, Day or Year is out of range
    Juldate = 0;
    return(Juldate);
  END; // if stuff is out of range

  M0 = M - 1;

  Juldate =  Y0 * 365 // Number of days in previous normal years
            + Y0 / 4   // Number of possible leap days
            - Y0 / 100 // Subtract all century years
            + Y0 / 400 // Add back the true leap century years
            + ADIPM[M0] + M0 * 30 + D;

  IF ((( Y MOD 4 EQ 0) AND ( Y MOD 100 NE 0)) OR ( Y MOD 400 EQ 0)) AND  (M > 2) THEN
//   123             3     3               32    2               21

    Juldate = Juldate + 1;
  END; // IF have to increment Juldate
  return Juldate;
END;// JULIAN

// --------------------------------------- GREGORIAN ----------------------------------------
RETURN MDYType FUNCTION GREGORIAN(int Juldate) {

  int Y0,M0,D0;
  const int MinJuldate = 630000;
  MDYType mdy;

  IF Juldate < MinJuldate THEN  // fixed this bug in Go version July 2016.  Get infinite loop if Juldate is too small.
      mdy.m = 0;
      mdy.d = 0;
      mdy.y = 0;
      return mdy;
  ENDIF;

  Y0 = Juldate / 365;
  M0 = 1;
  D0 = 1;

  while (JULIAN(M0,D0,Y0) > Juldate) LOOP Y0 = Y0 - 1; END;

  M0 = 12;
  while (JULIAN(M0,D0,Y0) > Juldate) LOOP M0 = M0 - 1; END;

  while (JULIAN(M0,D0,Y0) < Juldate) LOOP D0 = D0 + 1; END;

  mdy.m = M0;
  mdy.d = D0;
  mdy.y = Y0;
  return mdy;
};// GREGORIAN

// END timlibc

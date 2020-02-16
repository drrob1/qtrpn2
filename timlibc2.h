#ifndef macros
#define macros
#include "macros.h"
#endif
/*
  REVISION HISTORY
  ----------------
   6 Oct 13 -- Converted to gm2.  And changed its name.
  19 Jan 14 -- Converted to Ada.
  17 Nov 14 -- Converted to C++.
   7 Dec 14 -- Removed ElapsedSec from the exported data type.  See comment below.
  20 Jan 20 -- Started change to use int as standard type, not unsigned int. 
*/

/* tm is a c standard datatype.
  time_t t=time(0);  absolute time in seconds, or -1 if unknown
  tm POINTER p = gmtime( ADROF t);   usually indicated as tm* p = whatever
    sec,min,hour,mday,mon (0-11), year (subt 1900), wday, yday, isdst

struct tm IS
  tm_sec    int	seconds after the minute  0-60*
  tm_min    int	minutes after the hour    0-59
  tm_hour   int	hours since midnight      0-23
  tm_mday   int	day of the month          1-31
  tm_mon    int	months since January      0-11
  tm_year   int	years since 1900
  tm_wday   int	days since Sunday         0-6
  tm_yday   int	days since January 1      0-365
  tm_isdst  int	Daylight Saving Time flag
END


time_t ->  Time type
Alias of a fundamental arithmetic type capable of representing times, as those returned by function time.

For historical reasons, it is generally implemented as an integral value representing the number of seconds 
elapsed since 00:00 hours, Jan 1, 1970 UTC (i.e., a unix timestamp). Although libraries may implement this 
type using alternative time representations.

Portable programs should not use values of this type directly, but always rely on calls to elements of the 
standard library to translate them to portable types.


time

time_t time (time_t* timer);

Get current time
Get the current calendar time as a value of type time_t.

The function returns this value, and if the argument is not a null pointer, it also sets this value to the 
object pointed by timer.

The value returned generally represents the number of seconds since 00:00 hours, Jan 1, 1970 UTC (i.e., the 
current unix timestamp). Although libraries may use a different representation of time: Portable programs 
should not use the value returned by this function directly, but always rely on calls to other elements of 
the standard library to translate them to portable types (such as localtime, gmtime or difftime).

Parameters

timer
    Pointer to an object of type time_t, where the time value is stored.
    Alternatively, this parameter can be a null pointer, in which case the parameter is not used (the function 
    still returns a value of type time_t with the result).

int main ()
{
  time_t timer;
  struct tm y2k;
  double seconds;

  y2k.tm_hour = 0;   y2k.tm_min = 0; y2k.tm_sec = 0;
  y2k.tm_year = 100; y2k.tm_mon = 0; y2k.tm_mday = 1;

  time(&timer);  // get current time; same as: timer = time(NULL)

  seconds = difftime(timer,mktime(&y2k));

  printf ("%.f seconds since January 1, 2000 in the current timezone", seconds);

  return 0;
}

localtime

struct tm * localtime (const time_t * timer);

Convert time_t to tm as local time
Uses the value pointed by timer to fill a tm structure with the values that represent the corresponding time, 
expressed for the local timezone.

Parameters

timer
    Pointer to an object of type time_t that contains a time value.
    time_t is an alias of a fundamental arithmetic type capable of representing times as returned by function 
      time.


Return Value
A pointer to a tm structure with its members filled with the values that correspond to the local time 
representation of timer.

The returned value points to an internal object whose validity or value may be altered by any subsequent call 
to gmtime or localtime.


// localtime example
#include <stdio.h>     //  puts, printf 
#include <time.h>      //  time_t, struct tm, time, localtime

int main ()
{
  time_t rawtime;
  struct tm *timeinfo;

  time (&rawtime);
  timeinfo = localtime (&rawtime);
  printf ("Current local time and date: %s", asctime(timeinfo));

  return 0;
}

Output:
Current local time and date: Wed Feb 13 17:17:11 2013
	
strftime
size_t strftime (char* ptr, size_t maxsize, const char* format,
                 const struct tm* timeptr );

Format time as string
Copies into ptr the content of format, expanding its format specifiers into the corresponding values that represent the time described in timeptr, with a limit of maxsize characters.Parameters

ptr
    Pointer to the destination array where the resulting C string is copied.
maxsize
    Maximum number of characters to be copied to ptr, including the terminating null-character.

timeptr
    Pointer to a tm structure that contains a calendar time broken down into its components (see struct tm).


Return Value
If the length of the resulting C string, including the terminating null-character, doesn't exceed maxsize, the function returns the total number of characters copied to ptr (not including the terminating null-character).
Otherwise, it returns zero, and the contents of the array pointed by ptr are indeterminate.
*/
 
#ifndef timlibc2
#define timlibc2
  struct DateTimeType {
    time_t rawtime;
    tm TimeInfo;
    int month,day,year,hours,minutes,seconds;
  }; // DateTimeType Record

  struct MDYType {
    int m,d,y;
  };

  const string DAYNAMES[7] = {"Sunday","Monday","Tuesday","Wednesday", "Thursday","Friday","Saturday"};
  const string MONTHNAMES[12] = {"January","February","March","April","May", "June","July","August",
                                 "September","October","November","December"};

RETURN string FUNCTION MDY2STR(int M, int D, int Y, string REF MDYSTR);

RETURN MDYType FUNCTION TIME2MDY();
// System Time To Month, Day, and Year Conversion.

RETURN DateTimeType FUNCTION GetDateTime();
//            used to be this                 PROCEDURE GetDateTime(DateTimeType REF DateTime); 
// DateTimeType is my record time type containing everything.

RETURN int FUNCTION JULIAN(int M, int D, int Y);

RETURN MDYType FUNCTION GREGORIAN(int Juldate);

// timlibc2.h
#endif

#include <cstring>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <cctype>
#include <cmath>
#include <ctime>
#include <string>  // string class of c++
#include <iomanip>
using namespace std;  // this directive is default in gcc, and is basically ignored.  Needed under Windows.

// #ifndef macros   these screw up gvim code checking
// #define macros
#define EQ ==
#define NE !=
#define BEGIN
#define END }
#define ENDIF }
#define ENDFOR }
#define ENDCASE }
#define ENDLOOP }
#define ENDWHILE }
#define ENDDOO } while    // intended for the do { } while (expr); looping construct
#define DOWHILE } while   // also intended for do {} while (expr);
#define REPEAT do {          // This REPEAT ...  UNTIL of Modula-2 is same as do {} while (expr);
#define UNTIL(c) } while (NOT (c))    // So this is more readable than my other attempts.  
                                     // and I fixed this bug 30 Apr 15.
#define ENDFUNC }
#define ENDPROC }
#define MOD %
#define AND &&
#define OR ||
#define NOT !
#define then {
#define IF if (
#define THEN ){ 
#define ELSE }else{
#define ELSIF }else if ( 
#define IS {
#define OF {
#define DOO do {         // intended for the do {} while (expr); looping construct
#define DO ){
#define FOR for (
#define WHILE while (
#define SWITCH switch (
#define LOOP {
#define ADROF &
#define ADR &
#define DEREF *
#define POINTERTO(var)  * var
#define POINTER *
#define POINTEROF *
#define TOPOINTER *
#define TYPEOFPOINTER *
#define POINTINGBACKTO *
#define PointerType *
#define PtrType *
#define VAR &
#define REF &
#define PROCEDURE void
#define FUNCTION
#define RETURN
#define ARRAY
#define ARRAYOF
#define BITAND &
#define BITOR |
#define BITNOT ~
#define BITXOR ^
#define argcargv int argc, char *argv[]
#define ADDRESS void*  // typedef line is an error
#define COPY &
#define MOVE &&
#define TRY try{
#define BLOCK {
#define CAP(c) (char) toupper(c)  // int toupper ( int c );  The value is returned as an int value that can be implicitly casted to char.
// #endif


//#ifndef FileHandle
//#define FileHandle
typedef FILE* FileHandle;
typedef const char* PointerToConstCharType;
typedef char* CharPointerType;
typedef char* ARRAYOFCHAR;
typedef const char* CONSTARRAYOFCHAR;
typedef char* C_STRINGTYPE;
typedef const char* CONST_C_STRINGTYPE;
//#endif

#ifdef NCURSESON
#include <ncurses.h>
#include <panel.h>
typedef WINDOW* WinPtrType;
typedef PANEL* PanelPtrType;
#endif



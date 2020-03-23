#include <vector>
#include "macros.h"
#include <iomanip>
#include "getcommandline2.h"
#include "timlibc2.h"
#include "tokenizec2.h"
#include "hpcalcc2.h"
#include "holidaycalc2.h"


/* (C) 1990.  Robert W Solomon.  All rights reserved.
  REVISION HISTORY
  ----------------
   1 Dec 89 -- Added the help command.
  24 Dec 91 -- Converted to M-2 V 4.00.  Also changed the params to the
                GETRESULT proc to be more reasonable.
  21 Mar 93 -- Added exponentiation and MOD, INT, FRAC and ROUND, as well as used the UL2 procs again.
  25 Jul 93 -- Added JUL and GREG commands.
  18 May 03 -- Win32 version using Stony Brook Modula-2 v 4
  26 May 03 -- Allowed years to pivot for 2000 or 1900 in TIMLIB.
   4 Oct 03 -- Added LongCard2HexStr and its cmd.
  31 Oct 03 -- Added HCF cmd.
   1 Nov 03 -- Fixed the var swap bug in HCF rtn.
  27 Nov 04 -- Added pi as a cmd.
  12 Mar 06 -- Made trig fcn's use arguments in degrees, changed stacksize to const, and moved stackregnames to def.
  22 Jul 06 -- Added % operator does xy/100 but does not drop stack.  Just like old HP 25.
  15 Apr 07 -- Added comma to push stack.  Removed it as a delim char and made it a ALLELSE.  And
                added ! ` and ~ as stack commands.  And updated i/o procedure use.
   1 Apr 13 -- Modified the DUMP routine so the characters are not printed on the same line as the numbers.  The
                alignment was all wrong anyway.  And added Dump2Console.
   2 May 13 -- Will use consolemode compilation flags
  15 Oct 13 -- converting to gm2.
  28 Jun 14 -- converting to Ada.
  31 Aug 14 -- found that tknrtnsa does not process gettknreal correctly.  Had to rewrite it to match the Modula-2 version.
   2 Sep 14 -- Added ToHex back.
   1 Dec 14 -- Converted to cpp.
  20 Dec 14 -- Added use of macros for date of last compile
  31 Dec 14 -- Started coding HOL command, and changed the limits for the OutputFixedOrFloat proc to print Julian numbers in fixed format.
  10 Jan 15 -- Coding use of string operations to supplement output of stack.  And number cropping, addcommas.
   5 Nov 15 -- New commands for recip, curt, vol.
  22 Nov 15 -- Noticed that T1 and T2 stack operations are not correct.  This effects HP2cursed and rpnc.  Changed ARRAYOF Stack declaration.
                Was Stack[T1], now is Stack[StackSize].  The declaration is number of elements, not the high bound.
  13 Apr 16 -- Adding undo and redo commands, which operate on the entire stack not just X register.
  19 May 16 -- Fixing help text for the % commanded coded in 2006.  Oddly enough, the help never included it.
   2 Jul 16 -- Fixed help text for the PI command.  And changed the pivot for the JUL command to be the current year instead of the constant "30".
   7 Jul 16 -- Added UP command.  Surprising I had not done this earlier.
  22 Jan 20 -- Now called hpcalcc2.cpp, to update code and use vectors.
  25 Jan 20 -- Code is basically working.  Added sigfig, ? for help, and StackNames array.
  27 Jan 20 -- Made sigfig = -1 to see what that does.  It works like in Go.  I guess this is std behavior.
   2 Feb 20 -- Added fixN command to behave like sigN command.
   2 Feb 20 -- Started to add the prime and primefac command code, derived from Go.
   4 Feb 20 -- Fiddled with a format code for the prime cmd.
   7 Feb 20 -- Fixed a bug in greg cmd with regards to stack management.  And found an oddity on STACKDN, in that it does not alter X.  Don't really remember why not.
   8 Feb 20 -- Added PopX, after doing this in Go first.  Now that PopX works, I'm using it in other spots also.
                 And HCF now returns the HCF as a string without altering the stack.
                 UNDO is still not working.  I'm going to change stackmatrix operations so they do not do a rollup or rolldonw, instead just a move up and move down.
                 This file is now different than the version in ~/cppcode.
   9 Feb 20 -- Added that > or < will also swap X <--> Y.  And changed how DOW works, now it doesn't alter stack but returns a string message.
  22 Mar 20 -- Fixed bug in PrimeFac in that it needs to complain about zero values.

*/

/*
 Declared in the header file
  enum StackRegNames {X,Y,Z,T5,T4,T3,T2,T1,StackSize};
*/


ARRAYOF  double Stack[StackSize];
ARRAYOF  double StackMatrix[StackSize][StackSize];   // new matrix to hold the stacks.  Does not need to be a square matrix, but it is easier for me this way.
ARRAYOF const string StackNames[StackSize] = {"X:  ", "Y:  ", "Z:  ", "T5: ", "T4: ", "T3: ", "T2: ", "T1: "};

const double PI = 3.141592653589793;
double LastX=0.0;
double MemReg=0.0;
HolType Holidays;
//int sigfig = 9;
int sigfig = -1;


//-----------------------------------------------------------------------------------------------------------------------------

PROCEDURE STACKUP() IS
  int S;
  FOR S = T2; S >= X; --S DO
    Stack[S+1] = Stack[S];
  ENDFOR;
ENDFUNC;// STACKUP

//------------------------------------------------------
PROCEDURE STACKDN() IS
  int S;
  FOR S = Y; S < T1; ++S DO // note that this does not change X.  I don't remember why.  I regret that design choice now, as of 02/07/2020 09:30:32 PM
    Stack[S] = Stack[S+1];
  ENDFOR;
ENDFUNC; // STACKDN

//------------------------------------------------------
PROCEDURE STACKROLLDN() IS
  double TEMP;
  TEMP = Stack[X];
  Stack[X] = Stack[Y];
  STACKDN();
  Stack[T1] = TEMP;
ENDFUNC; // STACKROLLDN

//------------------------------------------------------
PROCEDURE PUSHX(double R) IS
  STACKUP();
  Stack[X] = R;
END; // PUSHX


// ----------------------------------------------------- PopX ---------------------
double FUNCTION PopX() {
    double x;
	x = Stack[X];
	for (int S = X; S < T1; S++) {
		Stack[S] = Stack[S+1];
	}
	return x;
} // PopX

//------------------------------------------------------
RETURN double FUNCTION READX() IS
  return Stack[X];
END; // READX
//------------------------------------------------------
PROCEDURE SWAPXY() IS
  double TEMP;
  TEMP = Stack[X];
  Stack[X] = Stack[Y];
  Stack[Y] = TEMP;
END; // SWAPXY
//------------------------------------------------------
PROCEDURE GETSTACK(double STK[]) IS // this works because array without params is a pointer.
  int S;
  FOR S = X; S <= T1; S++ DO
    STK[S] = Stack[S];
  ENDFOR;
END;// GETSTACK
//-----------------------------------------------------------------------------------------------------------------------------
RETURN string FUNCTION AddCommas(string instr) IS
  string outstr;
  int len,i,decptposn,NumberOfCommas;

  len = instr.length();
  decptposn = instr.find('.');
  IF (unsigned) decptposn EQ string::npos THEN  // decimal point not found
    instr = instr.append(".");
    i = len;
  ELSE
    i = decptposn;
  ENDIF;
  
  outstr = instr;
  NumberOfCommas = i/3;
  WHILE (NumberOfCommas > 0) AND (i > 3 ) DO
    i -= 3;
    outstr = outstr.insert(i,",");     // insert single comma declared as a c-string.
    --NumberOfCommas;
  ENDWHILE;
  return outstr;
ENDFUNC; // AddCommas
//-----------------------------------------------------------------------------------------------------------------------------

string CropNStr(string instr) {
  string outstr;
  int len,i;

  len = instr.length() - 1;
  i = len;
  WHILE (i >= 0) AND (instr.at(i) EQ '0') DO
    i--;
  ENDWHILE;
  outstr = instr;
  IF i < len THEN
    outstr.erase(i+1);  // erases from i-th position to the end of the string.
  ENDIF;

  i = 0;
  len = outstr.length();
  WHILE (i <= len) AND (outstr.at(i) EQ ' ') DO
    i++;
  ENDWHILE;
  IF i > 0 THEN
    outstr.erase(0,i);  // erases from the beginning to the i-th position.
  ENDIF

  return outstr;
}; // CropNStr

//-----------------------------------------------------------------------------------------------------------------------------
string CropStr(string instr) {
  string outstr;
  int len,i;

  len = instr.length() - 1;
  i = len;
  WHILE (i >= 0) AND (instr.at(i) EQ '0') DO
    i--;
  ENDWHILE;
  outstr = instr;
  IF i < len THEN
    outstr.erase(i+1);  // erases from i-th position to the end of the string.
  ENDIF;

  i = 0;
  len = outstr.length();
  WHILE (i <= len) AND (outstr.at(i) EQ ' ') DO
    i++;
  ENDWHILE;
  IF i > 0 THEN
    outstr.erase(0,i);  // erases from the beginning to the i-th position.
  ENDIF

  return outstr;
}; // CropStr

//-----------------------------------------------------------------------------------------------------------------------------
RETURN vector<string> FUNCTION DumpStackFixed() {
  int S;  // Stack counter
  string str;
  vector<string> stringslice;
  stringslice.clear();
  stringstream strngstrm[StackSize];

//  FOR S=X; S <= T1; S++ DO   wrong direction
  FOR S=T1; S >= X; S-- DO
    strngstrm[S].setf(ios::fixed);
    strngstrm[S].width(15);
    strngstrm[S].precision(sigfig);
    strngstrm[S] << left << Stack[S];

    str = to_string(Stack[S]);
    str = CropNStr(str);
//                                                          str = CropStr(str);
    IF Stack[S] > 10000 THEN
      str = AddCommas(str);
    ENDIF;

    string str0 = StackNames[S];
    str0 += strngstrm[S].str();
    str0 += "          ";
    str0 += str;
    stringslice.push_back(str0);
  ENDFOR;
  return stringslice;
}; // DumpStackFixed
// ***************************************************************************
RETURN vector<string> DumpStackFloat() IS
    // vector<string> stringslice; in header file
  int S;
  string str;
  vector<string> stringslice;
  stringslice.clear();
  stringstream strngstrm[StackSize];

//  FOR S=X; S <= T1; S++ DO   wrong direction
  FOR S=T1; S >= X; S-- DO
    strngstrm[S].flush();
    strngstrm[S].setf(ios::scientific);
    strngstrm[S].width(15);
    strngstrm[S].precision(sigfig);
    strngstrm[S] << left << Stack[S];

    str = to_string(Stack[S]);
    str = CropNStr(str);
//                                                          str = CropStr(str);
    IF Stack[S] > 10000 THEN
      str = AddCommas(str);
    ENDIF;

    string str0 = StackNames[S];
    str0 += strngstrm[S].str();
    str0 += "          ";
    str0 += str;
    stringslice.push_back(str0);
  ENDFOR;
  return stringslice;
END;// DumpStackFloat
//********************************************************************************
RETURN string FUNCTION ShowZeroIfSmall(double r) {
  string str;

  IF (r EQ 0.0) OR abs(r) < 1.0e-10 THEN  // write 0.0
     str = "0.0"; 
  ELSE
    str = to_string(r);
  ENDIF;
  return str;
}; // OutputFixedOrFloat
//*****************************************************************************
RETURN vector<string> FUNCTION DumpStackGeneral() {

  int S;
  string str;
  vector<string> stringslice;

  stringslice.clear();
//  FOR S=X; S <= T1; S++ DO   wrong direction
  FOR S=T1; S >= X; S-- DO
    str = to_string(Stack[S]);
//    str = CropNStr(str);
    str = CropStr(str);
    IF Stack[S] > 10000 THEN
      str = AddCommas(str);
    ENDIF;
    string str0 = StackNames[S];
    str0 += str;
    stringslice.push_back(str0);
  ENDFOR;
  return stringslice;
};// DumpStackGeneral

//-------------------------------------------------------------------

RETURN string FUNCTION ToHex(double L) {
  int K,Leng;
  unsigned int H;
  bool IsNeg;
  const char OrdZero='0';
  const char OrdCapA='A';
  string u_str1="",u_str2="";

  IF L < 0 THEN
    IsNeg = true;
    H = -L;
  ELSE
    IsNeg = false;
    H = L;
  ENDIF;

  REPEAT
    K = H MOD 16;
    IF (K <= 9) THEN
      u_str1 += (K + OrdZero);
    ELSE
      u_str1 += (K-10 + OrdCapA);
    ENDIF;
    H = H / 16;
  UNTIL(H EQ 0);

  IF IsNeg THEN
    u_str2 += '-';
  ENDIF;

  Leng = u_str1.length()-1;
  WHILE Leng >= 0 DO
    u_str2 += u_str1.at(Leng);
    Leng--;
  ENDWHILE;
  return u_str2;
};// ToHex
//--------------------------------------------------------
PROCEDURE StacksUp() {
  int i,j;
  FOR i=T2; i >= 0; i-- DO
    FOR j=X; j <= T1; j++ DO
      StackMatrix[i+1][j] = StackMatrix[i][j];
    ENDFOR; // FOR j
  ENDFOR;  // FOR i
}; // StacksUp
//--------------------------------------------------------
PROCEDURE StacksDown() {
  int i,j;
  FOR i=Y; i <= T1; i++ DO
    FOR j=X; j <= T1; j++ DO
      StackMatrix[i-1][j] = StackMatrix[i][j];
    ENDFOR; // FOR j
  ENDFOR;  // FOR i
}; // StacksDown
//--------------------------------------------------------
PROCEDURE PushStacks() {
  StacksUp();
  int j;
  FOR j=X; j <= T1; j++ DO
    StackMatrix[0][j] = Stack[j];
  ENDFOR;
}; // PushStacks
//--------------------------------------------------------
PROCEDURE PopStacks() {  // also works as an undo for main stack.
    for (int i = X; i < StackSize; i++) {
        Stack[i] = StackMatrix[0][i];
    }

    StacksDown();
}
//--------------------------------------------------------
PROCEDURE RollDownStacks() {    // undo operation for main stack
  int j;
  ARRAYOF  double TempStack[StackSize];

  FOR j=0; j <= T1; j++ DO
    TempStack[j] = Stack[j];
    Stack[j] = StackMatrix[0][j]; 
  ENDFOR;

  StacksDown();

  FOR j=0; j <= T1; j++ DO
    StackMatrix[T1][j] = TempStack[j]; 
  ENDFOR;
}; // RollDownStacks  IE undo
//--------------------------------------------------------
PROCEDURE RollUpStacks() {     // redo uperation for main stack
  int j;
  ARRAYOF  double TempStack[StackSize];

  FOR j=X; j <= T1; j++ DO
    TempStack[j] = Stack[j];
    Stack[j] = StackMatrix[T1][j];
  ENDFOR;

  StacksUp();

  FOR j=X; j <= T1; j++ DO
    StackMatrix[0][j] = TempStack[j];
  ENDFOR;
};  // RollUpStacks  IE redo
//--------------------------------------------------------
RETURN int FUNCTION HCF(int a, int b) {
// a = bt + r, then hcf(a,b) = hcf(b,r)
  int r,a1,b1;

  IF a < b THEN
    a1 = b;
    b1 = a;
  ELSE
    a1 = a;
    b1 = b;
  ENDIF;
  REPEAT
    r = a1 MOD b1;
    a1 = b1;
    b1 = r;
  UNTIL (r EQ 0);
  return a1;
};// HCF

/*  From Go.  Converting to C++. */
// ------------------------------------------------- IsPrime -----------------
//  func IsPrime(real float64) bool { // The real input is to allow from stack.
bool FUNCTION IsPrime(double real) { // The real input is to allow from stack.

	int t = 3;
	int RoundSqrt, Uint, Rsqrt;  // Uint was just that in Go, but I won't do that here.

	Uint = round(abs(real));

	if (Uint == 0 || Uint == 1) {
		return false;
	} else if (Uint == 2 || Uint == 3) {
		return true;
	} else if (Uint % 2 == 0) {
		return false;
	}

	Rsqrt = sqrt(real);
	RoundSqrt = round(Rsqrt);

	WHILE t <= RoundSqrt DO
		if (Uint % t == 0) {
			return false;
		}
		t += 2;
	ENDWHILE;
	return true;
} // IsPrime


// ------------------------------------------------- IsPrimeInt -----------------
//func IsPrimeInt(n int) bool {
bool FUNCTION IsPrimeInt(int n) {

	int t = 3;

	int Uint = n;

	IF Uint == 0 || Uint == 1 THEN
		return false;
    ELSIF Uint == 2 || Uint == 3 THEN
		return true;
	ELSIF Uint%2 == 0 THEN
		return false;
	ENDIF

    int Sqrt = sqrt( (double) Uint);
	int UintSqrt = (int) Sqrt;

	WHILE t <= UintSqrt DO
		IF Uint%t == 0 THEN
			return false;
		ENDIF;
		t += 2;
	ENDWHILE;
	return true;
} // IsPrimeInt

// ------------------------------------------------- PrimeFactorization ---------------------------------
//func PrimeFactorization(N int) []int {
vector<int> FUNCTION PrimeFactorization(int N) {
	ARRAYOF int PD[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47}; // Prime divisors array
    vector<int> PrimeFactors;
    int n;

    PrimeFactors.clear();

	n = N;
    int lenPD = (int) sizeof(PD);
	FOR int i = 0; i < lenPD; i++ DO // outer loop to sequentially test the prime divisors
		WHILE n > 0 && n%PD[i] == 0 DO
			PrimeFactors.push_back(PD[i]);
			n = n / PD[i];
		ENDWHILE;
		IF n == 0 || IsPrimeInt(n) THEN
			PrimeFactors.push_back(n);
			break;
		ENDIF
	ENDFOR;
	return PrimeFactors;

} // PrimeFactorization

//----------------------------------------------- usqrt ---------------------------
//func usqrt(u uint) uint {
int FUNCTION usqrt(int u) {

	int Sqrt = u / 2;

	FOR int i = 0; i < 30; i++ DO
		int guess = u / Sqrt;
		Sqrt = (guess + Sqrt) / 2;
		IF Sqrt-guess <= 1 THEN // recall that this is not floating math.
			break;
		ENDIF;
	ENDFOR;
	return Sqrt;
}

// ------------------------------------------------- NextPrimeFac -----------------
//func NextPrimeFac(n, startfac uint) (uint, bool) { // note that this is the reverse of IsPrime
bool FUNCTION NextPrimeFac(int n, int REF startfac) { // note that this is the reverse of IsPrime

	int t = startfac;

	int UintSqrt = usqrt(n);

	WHILE t <= UintSqrt DO
		IF n % t == 0 THEN
            startfac = t;  // in Go, this was returned part of a tuple.  Not allowed in C++.
			return true;
		ENDIF;
		IF t == 2 THEN
			t = 3;
		ELSE 
			t += 2;
		ENDIF;
	ENDWHILE;
    startfac = 0;  // in Go, this was returned part of a tuple.  Not allowed in C++.
	return false;
} // IsPrime

// --------------------------------------- PrimeFactorMemoized -------------------
//func PrimeFactorMemoized(U uint) []uint {
vector<int> FUNCTION PrimeFactorMemoized(int U) {

    vector<int> PrimeUfactors;

    PrimeUfactors.clear();

	if (U == 0) {
		return PrimeUfactors;
	}

	int fac = 2;

	FOR int i = U; i > 1; DO
		bool facflag = NextPrimeFac(i, fac); // fac is a REF param
		if (facflag) {
			PrimeUfactors.push_back(fac);
			i = i / fac;
		} else {
			PrimeUfactors.push_back(i);
			break;
		}
	ENDFOR;
	return PrimeUfactors;
} // PrimeFactorMemoized
//-------------------------------------------------------------------------
// I'm thinking of returning a StackMatrix row as a vector<double>.  The row is an input param.
vector<double> FUNCTION GetStackMatrixRow(int i) {
    vector<double> StackMatrixRow;
    StackMatrixRow.clear();

    IF i > StackSize THEN
            return StackMatrixRow;
    ENDIF

    for (int j = X; j < StackSize; j++) {
        StackMatrixRow.push_back(StackMatrix[i][j]);
    }
    return StackMatrixRow;
}
//-------------------------------------------------------------------------
// Do I even need to return the Stack as a vector<double>?  This duplicates GETSTACK.
//-------------------------------------------------------------------------
calcPairType FUNCTION GetResult(string s) {
/*
  struct calcPairType {  // defined in header file
    vector<string> ss;
    double R;
  }
*/

  int I,year;
  TokenType Token;
  bool EOL;
  calcPairType calcpair;

  calcpair.ss.clear();
  calcpair.R = 0.0;

  INITKN(s);
  WHILE true DO //  UNTIL reached EOL
    EOL = GETTKNREAL(Token);
    IF EOL THEN break; ENDIF;

    I = Token.iSum;
    SWITCH  Token.STATE DO
      case DELIM : break; /* do nothing */
      case DGT : 
                 PUSHX(Token.rSum);
                 PushStacks();
                 break;
      case OP  : IF (I EQ 6) OR (I EQ 20) OR (I EQ 1) OR (I EQ 3) THEN  // allow <>, ><, <, > to all call SWAPXY().
      	            SWAPXY();
                 ELSE
                    LastX = Stack[X];
                    PushStacks();
                    SWITCH I DO
                      case  8 : Stack[X] += Stack[Y];
                                 break;
                      case 10 : Stack[X] = Stack[Y] - Stack[X];
                                 break;
                      case 12 : Stack[X] *= Stack[Y];
                                 break;
                      case 14 : Stack[X] = Stack[Y] / Stack[X];
                                 break;
                      case 16 : Stack[X] = pow(Stack[Y], round(Stack[X])); // ^ op
                                 break;
                      case 18 : Stack[X] = pow(abs(Stack[Y]), Stack[X]); // **
                                 break;
                      case 22 : Stack[X] *= Stack[Y] / 100.0;  // percent
                                 break;
                      default :
                                 cerr << Token.uStr + " is an unrecognized operation." << endl;
                                 STACKUP();
                    ENDCASE; // case on opcode
                    IF I NE 22 THEN STACKDN(); ENDIF; // Do not move stack for % operator
                 ENDIF; // opcode value condition
                 break;
    case ALLELSE :  IF false THEN /* do nothing */
                    ELSIF Token.uStr.compare("DUMP") EQ 0 THEN
                      calcpair.ss = DumpStackGeneral();
                    ELSIF Token.uStr.compare("SHO") EQ 0 THEN
                      calcpair.ss = DumpStackGeneral();
                    ELSIF Token.uStr.find("DUMPFIX") EQ 0 THEN  // will also match dumpfixed
                      calcpair.ss = DumpStackFixed();
                    ELSIF Token.uStr.compare("DUMPFLOAT") EQ 0 THEN
                      calcpair.ss = DumpStackFloat();
                    ELSIF (Token.uStr.find("SIG") EQ 0) OR (Token.uStr.find("FIX") EQ 0) THEN // if found at posn zero, ie string begins with 
                      IF Token.uStr.length() == 3 THEN
                        sigfig = 9;
                      ELSE
                        int last = Token.uStr.back();
                        last -= '0';
                        IF (last >= 0) AND (last <= 9) THEN
                          sigfig = last;
                        ELSE
                          // sigfig = 9;
                          sigfig = -1;
                        END;
                      END;
                    ELSIF Token.uStr.compare("PRIME") EQ 0 THEN
                      PushStacks();
                      int n = round(Stack[X]);
                      IF IsPrime(n) THEN
                          char s[50];
                          sprintf(s,"%10.0f is prime.",Stack[X]);
                          string str = s;
                          calcpair.ss.push_back(str);
                      ELSE
                          char s[50];
                          sprintf(s,"%10.0f is not prime.",Stack[X]);
                          string str = s;
                          calcpair.ss.push_back(str);
                      ENDIF;

                    ELSIF Token.uStr.find("PRIMEF") EQ 0 THEN
                      PushStacks();
                      string str;
                      int n = round(Stack[X]);
                      if (n < 2) {
                          str = " PrimeFac cmd of numbers < 2 ignored.";
                          calcpair.ss.push_back(str);
                      } else {

                          vector<int> primefactors = PrimeFactorMemoized(n);
                          vector<int>::iterator primeit;

                          // always will have as factors 1 and itself.  Cannot get an empty set of factors.

                          FOR primeit = primefactors.begin(); primeit != primefactors.end(); primeit++ DO
                            char s[50];
                            sprintf(s, "%d", *primeit);
                            string s1 = s;
                            str.append(s1);
                            str.append(", ");
                          ENDIF;
                          str.pop_back();  // delete the last ", "
                          str.pop_back();  // delete the last ", "
                          calcpair.ss.push_back(str);
                      }

                    ELSIF Token.uStr.find("POP") EQ 0 THEN // allow POP, POPX, or even POPULAR :-)
                        PushStacks();
                        double x = PopX();
                        string str = to_string(x);
                        calcpair.ss.push_back(str);

                    ELSIF Token.uStr.compare("SQR") EQ 0 THEN
                      LastX = Stack[X];
                      PushStacks();
                      Stack[X] *= Stack[X];
                    ELSIF Token.uStr.compare("SQRT") EQ 0 THEN
                      LastX = Stack[X];
                      PushStacks();
                      Stack[X] = sqrt(Stack[X]);
                    ELSIF Token.uStr.compare("RECIP") EQ 0 THEN
                      LastX = Stack[X];
                      PushStacks();
                      Stack[X] = 1.0/Stack[X];
                    ELSIF Token.uStr.compare("CURT") EQ 0 THEN
                      LastX = Stack[X];
                      PushStacks();
                      // Stack[X] = exp(log(Stack[X])/3.0); old way
                      Stack[X] = cbrt(Stack[X]);
                    ELSIF Token.uStr.compare("VOL") EQ 0 THEN
                      LastX = Stack[X];
                      PushStacks();
                      Stack[X] = exp(log(2.0*Stack[X])/3.0);
                    ELSIF (Token.uStr.compare("HELP") EQ 0) OR (Token.uStr.compare("?") EQ 0) THEN
                      calcpair.ss.push_back( " ABOUT -- indicate compilation times.");
                      calcpair.ss.push_back( " SQRT,SQR -- X = sqrt(X) or sqr(X) register.");
                      calcpair.ss.push_back(" CURT -- X = cuberoot(X).");
                      calcpair.ss.push_back( " RECIP -- X = 1/X.");
                      calcpair.ss.push_back(" VOL -- X = estimated diameter for a given volume, assuming a sphere.");
                      calcpair.ss.push_back(" STO,RCL  -- store/recall the X register to/from the memory register.");
                      calcpair.ss.push_back(" `,~,SWAP,SWAPXY,<>,>< -- equivalent commands that swap the X and Y registers.");
                      calcpair.ss.push_back(" @, LastX -- put the value of the LASTX register back into the X register.");
                      calcpair.ss.push_back(" !,ROLLDN -- roll the stack down one register.  X goes to T1.");
                      calcpair.ss.push_back(" pop, dn -- remove X from the stack amd move the rest down one register.");
                      calcpair.ss.push_back(" , or UP -- stack up.  ! or DN -- stack down.");
                      calcpair.ss.push_back(" Dump, Dumpfixed, Dumpfloat, Sho -- dump the stack to the terminal.");
                      calcpair.ss.push_back(" sigN, fixN -- set the sigfig amount, range 0..9");
                      calcpair.ss.push_back(" prime, primefac -- calls IsPrime and PrimeFacMemoized.");
                      calcpair.ss.push_back(" EXP,LN -- evaluate exp(X) or ln(X) and put result back into X.");
                      calcpair.ss.push_back(" ^  -- ABS(Y) to the X power, put result in X and pop stack 1 reg.  Rounds X");
                      calcpair.ss.push_back(" **  -- ABS(Y) to the X power, put result in X and pop stack 1 reg.");
                      calcpair.ss.push_back(" INT, ROUND, FRAC, PI -- do what their names suggest.");
                      calcpair.ss.push_back(" MOD -- evaluate Y MOD X, put result in X and pop stack 1 reg.");
		              calcpair.ss.push_back(" %   -- does XY/100, places result in X.  Leaves Y alone.");
                      calcpair.ss.push_back(" SIN,COS,TAN,ARCTAN,ARCSIN,ARCCOS -- In deg.");
                      calcpair.ss.push_back(" D2R -- perform degrees to radians conversion of the X register.");
                      calcpair.ss.push_back(" R2D -- perform radians to degrees conversion of the X register.");
                      calcpair.ss.push_back(" JUL -- Return Julian date number of Z month, Y day, X year.  Pop stack x2.");
                      calcpair.ss.push_back(" GREG-- Return Z month, Y day, X year of Julian date number in X.");
                      calcpair.ss.push_back(" TODAY- Return Julian date number of today's date.  Pop stack x2.");
                      calcpair.ss.push_back(" DOW -- Return day number 0..6 of julian date number in X register.");
                      calcpair.ss.push_back(" HEX -- Round X register to a long_integer and output it in hex format.");
                      calcpair.ss.push_back(" HCF -- Push HCF(Y,X) onto stack without removing Y or X.");
                      calcpair.ss.push_back(" HOL -- Display holidays.");
                      calcpair.ss.push_back(" UNDO, REDO -- entire stack.  More comprehensive than lastx.");
                      string s = " hpcalcc2 was last compiled ";
                      s.append(__DATE__);
                      s.append("  ");
                      s.append(__TIME__);
                      s.append(".");
                      calcpair.ss.push_back(s);
                    ELSIF Token.uStr.compare("STO") EQ 0 THEN
                      MemReg = Stack[X];
                    ELSIF Token.uStr.compare("RCL") EQ 0 THEN
                      PUSHX(MemReg);
                    ELSIF Token.uStr.compare("UNDO") EQ 0 THEN
                      RollDownStacks();
                      //      PopStacks();
                    ELSIF Token.uStr.compare("REDO") EQ 0 THEN
                      RollUpStacks();
                    ELSIF Token.uStr.compare("SWAP") EQ 0 THEN
                      PushStacks();
                      SWAPXY();
                    ELSIF Token.uStr.compare("SWAPXY") EQ 0 THEN
                      PushStacks();
                      SWAPXY();
                    ELSIF Token.uStr.compare("LASTX") EQ 0 THEN
                      PushStacks();
                      PUSHX(LastX);
                    ELSIF Token.uStr.compare("ROLLDN")  EQ 0 THEN
                      PushStacks();
                      STACKROLLDN();
                    ELSIF Token.uStr.compare("DN")  EQ 0 THEN
                      PushStacks();
                      PopX();
                    ELSIF (Token.uStr.compare(",") EQ 0) OR (Token.uStr.compare("UP") EQ 0) THEN
                      PushStacks();
                      STACKUP();
                    ELSIF Token.uStr.compare("!") EQ 0  THEN
                      PushStacks();
                      Stack[X] = Stack[Y];
                      PopX();
                    ELSIF Token.uStr.compare("`") EQ 0  THEN
                      PushStacks();
                      SWAPXY();
                    ELSIF Token.uStr.compare("~") EQ 0  THEN
                      PushStacks();
                      SWAPXY();
                    ELSIF Token.uStr.compare("@") EQ 0  THEN
                      PushStacks();
                      PUSHX(LastX);
                    ELSIF Token.uStr.compare("EXP") EQ 0  THEN
                      PushStacks();
                      LastX = Stack[X];
                      Stack[X] = exp(Stack[X]);
                    ELSIF Token.uStr.compare("LN") EQ 0  THEN
                      PushStacks();
                      LastX = Stack[X];
                      Stack[X] = log(abs(Stack[X]));  // log is natural log
                    ELSIF Token.uStr.compare("Y^X") EQ 0  THEN // Now that ^ is an operator, this must be quoted to be parsed as such
                      PushStacks();
                      LastX = Stack[X];
                      Stack[X] = exp(Stack[X]*log(abs(Stack[Y])));
                      STACKDN();
                    ELSIF Token.uStr.compare("INT") EQ 0  THEN
                      PushStacks();
                      LastX = Stack[X];
                      Stack[X] = trunc(Stack[X]);
                    ELSIF Token.uStr.compare("TRUNC") EQ 0  THEN
                      PushStacks();
                      LastX = Stack[X];
                      Stack[X] = trunc(Stack[X]);
                    ELSIF Token.uStr.compare("ROUND") EQ 0  THEN
                      PushStacks();
                      LastX = Stack[X];
                      Stack[X] = round(LastX);
                    ELSIF Token.uStr.compare("HEX") EQ 0  THEN
                      IF (Stack[X] >= -2.0E9) AND (Stack[X] <= 1.80E19) THEN
                        string s = " Value of X reg in hex: ";
                        s.append(ToHex(Stack[X]));
                        calcpair.ss.push_back(s);
                      ELSE
                        calcpair.ss.push_back(" Cannot convert X register to hex string, as number is out of range.");
                      ENDIF; // Hex command
                    ELSIF Token.uStr.compare("HCF") EQ 0  THEN
                      int c,c1,c2; 
//                                                             PushStacks();
                      c1 = abs(round(Stack[X]));
                      c2 = abs(round(Stack[Y]));
                      c  = HCF(c2,c1);
                      string s = "HCF between X and Y is ";
                      string r = to_string(c);
                      s.append(r);
                      calcpair.ss.push_back(s);
//                                                             STACKUP();
//                                                             Stack[X] = c;
                    ELSIF Token.uStr.compare("P") EQ 0  THEN
                      //  essentially do nothing but print RESULT= line again.
                    ELSIF Token.uStr.compare("FRAC") EQ 0  THEN
                      PushStacks();
                      LastX = Stack[X];
                      Stack[X] -= trunc(Stack[X]);
                    ELSIF Token.uStr.compare("MOD") EQ 0  THEN
                      PushStacks();
                      LastX = Stack[X];
                      Stack[X] = abs(lround(Stack[Y])) MOD abs(lround(Stack[X]));
                      STACKDN();
                    ELSIF Token.uStr.compare("SIN") EQ 0  THEN
                      PushStacks();
                      LastX = Stack[X];
                      Stack[X] = sin(Stack[X]*PI/180.0);
                    ELSIF Token.uStr.compare("COS") EQ 0  THEN
                      PushStacks();
                      LastX = Stack[X];
                      Stack[X] = cos(Stack[X]*PI/180.0);
                    ELSIF Token.uStr.compare("TAN") EQ 0  THEN
                      PushStacks();
                      LastX = Stack[X];
                      Stack[X] = tan(Stack[X]*PI/180.0);
                    ELSIF Token.uStr.compare("ARCTAN") EQ 0  THEN
                      PushStacks();
                      LastX = Stack[X];
                      Stack[X] = atan(Stack[X])*180.0/PI;
                    ELSIF Token.uStr.compare("ARCSIN") EQ 0  THEN
                      PushStacks();
                      LastX = Stack[X];
                      Stack[X] = asin(LastX)*180.0/PI;
                    ELSIF Token.uStr.compare("ARCCOS") EQ 0 THEN
                      PushStacks();
                      LastX = Stack[X];
                      Stack[X] = acos(LastX)*180.0/PI;
                    ELSIF Token.uStr.compare("D2R") EQ 0 THEN
                      PushStacks();
                      LastX = Stack[X];
                      Stack[X] *= PI/180.0;
                    ELSIF Token.uStr.compare("R2D") EQ 0 THEN
                      PushStacks();
                      LastX = Stack[X];
                      Stack[X] *= 180.0/PI;
                    ELSIF Token.uStr.compare("JUL") EQ 0 THEN
                      PushStacks();
                      LastX = Stack[X];
                         // allow for 2 digit years
                      MDYType mdy = TIME2MDY();
                      IF Stack[X] <= (mdy.y MOD 100) THEN // if <= current year last 2 digits
                      	Stack[X] += 2000.0;
                      ELSIF Stack[X] < 100.0 THEN
                      	Stack[X] += 1900.0;
                      ENDIF;
                      Stack[X] = JULIAN(lround(Stack[Z]), lround(Stack[Y]), lround(Stack[X]));
                      STACKDN();
                      STACKDN();
                    ELSIF Token.uStr.compare("TODAY") EQ 0 THEN
                      PushStacks();
                      LastX = Stack[X];
                      MDYType mdy = TIME2MDY();
                      PUSHX(JULIAN(mdy.m,mdy.d,mdy.y));
                    ELSIF Token.uStr.compare("GREG") EQ 0 THEN
                      PushStacks();
                      LastX = Stack[X];
                      MDYType mdy = GREGORIAN(round(Stack[X]));
                      PopX();  // added Feb 8, 2020.
                      PUSHX(mdy.m);  // new way of handing this
                      PUSHX(mdy.d);
                      PUSHX(mdy.y);
                    ELSIF Token.uStr.compare("DOW") EQ 0 THEN
                      // PushStacks();
                      // LastX = Stack[X];
                      // Stack[X] = (lround(LastX) MOD 7);
                      int dow = lround(Stack[X]) MOD 7;
                      char s[50];
                      sprintf(s," dow is %d, Juldate in X is %ld, which is a %s.",dow, lround(Stack[X]), DAYNAMES[dow].c_str());
                      string str = s;
                      calcpair.ss.push_back(str);

                    ELSIF Token.uStr.compare("PI") EQ 0 THEN
                      PushStacks();
                      PUSHX(PI);
                    ELSIF Token.uStr.compare("HOL") EQ 0 THEN
                      PushStacks();
                      year = round(Stack[X]);
                      IF year < 30 THEN
                        year += 2000;
                      ELSIF year < 100 THEN
                        year += 1900;
                      ENDIF;
                      stringstream strngstrm1, strngstrm2, strngstrm3;
                      IF (year >= 1900) AND (year <= 2100) THEN
                        HolType Holiday = GetHolidays(year);
                        
                          // DAYNAMES and MONTHNAMES are declared in timlibc2.h,
                          // included above
					    int NYD = JULIAN(1, 1, year) % 7;
                        strngstrm1 << "New Years Day is a " << DAYNAMES[NYD] << ", MLK Day is January " << Holiday.MLK.d << ", Pres Day is February " << Holiday.Pres.d << ", Easter Sunday is " << MONTHNAMES[Holiday.Easter.m]<< " " << Holiday.Easter.d << ", Mothers Day is May " << Holiday.Mother.d;
                        calcpair.ss.push_back(strngstrm1.str());
//                        strngstrm.clear();  didn't work


                        int July4 = JULIAN(7,4,year) MOD 7;
                        strngstrm2 << "Memorial Day is May " << Holiday.Memorial.d << ", Fathers Day is June " << Holiday.Father.d << ", July 4 is a " << DAYNAMES[July4] << ", Labor Day is Sept " << Holiday.Labor.d << ", Columbus Day is Oct " << Holiday.Columbus.d;
                        calcpair.ss.push_back(strngstrm2.str());
//                        strngstream.clear();  didn't work

                        int VetD = JULIAN(11,11,year) MOD 7;
                        int ChristmasD = JULIAN(12,25,year) MOD 7;
                        strngstrm3 << "Election Day is Nov " << Holiday.Election.d << ", Veterans Day is a " << DAYNAMES[VetD] << ", Thanksgiving is Nov " << Holiday.Thanksgiving.d << "and Christmas Day is a " << DAYNAMES[ChristmasD];
                        calcpair.ss.push_back(strngstrm3.str());
//                        strngstream.clear();  didn't work
                      ELSE
                        strngstrm1 << "Value in X register is not a valid year.  Command ignored.";
                        calcpair.ss.push_back(strngstrm1.str());
                      ENDIF;
                    ELSIF Token.uStr.compare("ABOUT") EQ 0 THEN
                      string s = " hpcalcc2 was last compiled ";
                      s.append(__DATE__);
                      s.append("  ");
                      s.append(__TIME__);
                      s.append(".");
                      calcpair.ss.push_back(s);
                    ELSE
                      cerr << Token.uStr + " is an unrecognized command." << endl;
                    ENDIF;  // main text command selection if statement
    ENDCASE;
  ENDWHILE;
  calcpair.R = Stack[X];
  return calcpair;
}; // GETRESULT



// end hpcalcc2.cpp

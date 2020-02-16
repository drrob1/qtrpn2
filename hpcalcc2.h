// (C) 1990.  Robert W Solomon.  All rights reserved.
// This module simulates an RPN calculator, but with an 8 reg stack instead of 4.  All operations occur at the bottom of the stack
#include "macros.h"
#include "timlibc2.h"
#include <vector>
#include <string>

  enum StackRegNames {X,Y,Z,T5,T4,T3,T2,T1,StackSize};  // StackSize is now same as T1 + 1
//  const int StackSize = T1 + 1;


  struct calcPairType {
    vector<string> ss;
    double R;
  };
 
RETURN string FUNCTION CropNStr(string instr);
//string CropNStr(string instr);
PROCEDURE PUSHX(double R);
string CropStr(string instr);
RETURN string FUNCTION AddCommas(string instr);
RETURN string FUNCTION ShowZeroIfSmall(double r);
RETURN double FUNCTION READX();
PROCEDURE GETSTACK(double STK[]);   // this works because array without params is a pointer.
RETURN vector<string> FUNCTION DumpStackFloat();
RETURN vector<string> FUNCTION DumpStackFixed();
RETURN vector<string> FUNCTION DumpStackGeneral();
PROCEDURE PushStacks();
PROCEDURE RollDownStacks();
PROCEDURE RollUpStacks();
RETURN string FUNCTION ToHex(double L);
RETURN calcPairType FUNCTION GetResult(string s);
vector<double> FUNCTION GetStackMatrixRow(int i);

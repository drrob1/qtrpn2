#include "tokenizec2.h"
/*
 Copyright (C) 1987-2014  Robert Solomon MD.  All rights reserved.
 These routines collectively implement a very good facility to fetch, manipulate, and interpret tokens.

  REVISION HISTORY
  ----------------
  28 MAY 87 -- Added UNGETTKN capability and no longer exported GETCHR and UNGETCHR.
  29 AUG 87 -- Restored exportation of GETCHR and UNGETCHR.
   3 Mar 88 -- Added the ASCZERO declaration and removed the function call from the DGT conversion loop.
  31 Mar 88 -- Converted to M2 V3.03.
  1 Sept 88 -- 1.  Allowed quoted string to force ALLELSE state.
               2.  Changed the method UNGETTKN uses to unget the token.
               3.  Added the MULTSIGN and DIVSIGN OP states.
               4.  Ran M2CHECK and deleted all unreferenced var's.
               5.  Moved the NEGATV check for contigently making SUM < 0 out
                   of the LOOP and deleted the 5 previous statements for all
                   of the different states plus the end-of-line test.
  18 Mar 89 -- Added the GETTKNREAL Procedure.
  20 Mar 89 -- Changed GETOPCODE so that if a multicharacter op code is
                invalid, UNGETCHR is used to put the second char back.
   1 Dec 89 -- Made change in GETTKN that was demonstrated to be necessary
                when the code was ported to the VAX.
   2 Jan 90 -- Changed GETTKNREAL so that a real number may begin with a decimal pt.
   9 Nov 90 -- Added GETTKNSTR procedure and DELIMSTATE var.
  27 Dec 90 -- Added GETTKNEOL procedure, originally written for CFNTS.
  25 Jul 93 -- Fixed bug in GETCHR whereby CHRSTATE not set when at EOL, and adjusted algorithm of GETTKNSTR.
   6 Jun 95 -- Added FSAARRAY as way to assign FSATYP, and to easily modify the FSATYP assignments.
  20 Oct 02 -- Converted to M2 for win32, DOS mode.
  17 May 03 -- First Win32 version.
  30 Jun 03 -- Fixed real tokens so can now again begin w/ decpt, by always writing a leading 0.
  21 Jul 03 -- Fixed bug introduced by above step when a token has leading spaces
   4 Oct 03 -- Fixed bug when neg number is entered using unary minus.
   9 Oct 13 -- Converted to gm2.
  11 Oct 13 -- Fixed a bug in GETTKNREAL in which number like 1e-1 lost the e.
  12 Oct 13 -- Removed an errant RETURN from GETTKNSTR.
   3 Feb 14 -- Converted to Ada.  I modernized the data types to be a record type.
  28 Jun 14 -- Backported enhancement to GetOpCode that includes ^, ** and %. 
  19 Nov 14 -- Converted to C++.
   7 Dec 14 -- Removed comma as a delim, making it AllElse so it works as intended for HPCALCC
  28 Dec 14 -- Turns out that CentOS c++ does not support -std=c++11, so I have to remove string.front and string.back 
                 member functions.
  18 Jan 15 -- Found bug in that single digits followed by add or subtract are not processed correctly by GETTKNREAL.
  24 Jan 20 -- Converting to tokenizec2 and using a more Go like syntax to head for Qt.
  10 Feb 20 -- Compiler flagged use of dgtval as possibly uninitialized when used.  So I initialized it.
*/

/*   Declared in tokenizec2.h
  enum FSATYP {DELIM, OP, DGT, ALLELSE};
  struct TokenType { 
    string uStr;
    FSATYP STATE;
    char delimCH;
    FSATYP DelimState;
    int iSum;
    double rSum;
  }; // TokenType record
  struct CharType {
    char ch;
    FSATYP STATE;
  }; // CharType Record

*/


      const int TKNMAXSIZ = 80;
      const int OPMAXSIZ  = 2;
      const int Dgt0 = '0';
      const int Dgt9 = '9';
      const int  ASCZERO  = (int) '0'; // 48, 0x30
      const int POUNDSIGN = '#';  /* 35 */
      const int PLUSSIGN  = '+';  /* 43 */
      const int COMMA = ',';  /* 44 */
      const int MINUSSIGN = '-';  /* 45 */
      const int SEMCOL = ';';  /* 59 */
      const int LTSIGN = '<';  /* 60 */
      const int EQUALSIGN = '=';  /* 61 */
      const int GTSIGN    = '>';  /* 62 */
      const int MULTSIGN  = '*';
      const int DIVSIGN   = '/';
      const char SQUOTE   = '\047'; /* 39 */
      const int DQUOTE   = '"'; /* 34 */
      const int NullChar = '\0';
      const int EXPSIGN  = '^';
      const int PERCNT   = '%';

// These variables are declared here to make the variable global so to maintain their values btwn calls.

  int CURPOSN,HOLDCURPOSN,PREVPOSN;

  string TKNBUF,HOLDTKNBUF;
  FSATYP FSAARRAY[256];

//  out_of_range, token_too_long : EXCEPTION  ;

char FUNCTION Cap(char c) IS
  return toupper(c);
ENDFUNC; // Cap

PROCEDURE InitFSAArray() IS
  FOR int i=0; i < 256; i++ DO
    FSAARRAY[i] = ALLELSE;
  ENDFOR;
  FOR int c=Dgt0; c <= Dgt9; c++ DO
    FSAARRAY[c] = DGT;
  ENDFOR;
  FSAARRAY[NullChar] = DELIM;
  FSAARRAY[' '] = DELIM;
/*  FSAARRAY[','] = DELIM;  No longer a delim */
  FSAARRAY[';'] = DELIM;
  FSAARRAY['#'] = OP;
  FSAARRAY['*'] = OP;
  FSAARRAY['+'] = OP;
  FSAARRAY['-'] = OP;
  FSAARRAY['/'] = OP;
  FSAARRAY['<'] = OP;
  FSAARRAY['='] = OP;
  FSAARRAY['>'] = OP;
  FSAARRAY['%'] = OP;
  FSAARRAY['^'] = OP;
  TKNBUF = "";
  HOLDTKNBUF = "";
ENDPROC; // InitFSAArray

// ***************************************** INITKN *******************************************
PROCEDURE INITKN(string Str) IS
//INITIALIZE TOKEN.
//THE PURPOSE OF THE INITIALIZE TOKEN ROUTINE IS TO INITIALIZE THE
//VARIABLES USED BY NXTCHR TO BEGIN PROCESSING A NEW LINE.
//THE BUFFER ON WHICH THE TOKENIZING RTNS OPERATE IS ALSO INITIALIZED.
//CURPOSN IS INITIALIZED TO START AT THE FIRST CHARACTER ON THE LINE.
//  Former name was INI1TKN
  InitFSAArray();
  CURPOSN  = 0;
  TKNBUF   = Str;
  PREVPOSN = 0;
  HOLDCURPOSN = 0;
  HOLDTKNBUF = "";

//    cout << " In IniTkn and TKNBUF is '";
//    cout << TKNBUF;
//    cout << "', length of TKNBUF is ";
//    cout << TKNBUF.length() << "." << endl;
//    cout << " Still in IniTkn and Str is '" << Str << "', length= " << Str.length() << endl;
ENDPROC; // INITKN

//****************************** STOTKNPOSN ***********************************
PROCEDURE STOTKNPOSN() IS
/*
 STORE TOKEN POSITION.
 THIS ROUTINE WILL STORE THE VALUE OF THE CURPOSN INTO A HOLD VARIABLE FOR
 LATER RECALL BY RCLTKNPOSN.  ITS MAIN USE IS BY THE GET ENVIRONMENT PROC'S
 SO THAT THE ENVIRONMENT NEED BE PARSED ONLY ONCE.
*/

  IF (CURPOSN < 0) OR ( (unsigned) CURPOSN > TKNBUF.length()) THEN
    throw string(" out_of_range error from StoTknPosn");
  ENDIF;
  HOLDCURPOSN = CURPOSN;
  HOLDTKNBUF = TKNBUF;
ENDPROC; // STOTKNPOSN

//****************************** RCLTKNPOSN **********************************
PROCEDURE RCLTKNPOSN() IS
/*
 RECALL TOKEN POSITION.
 THIS IS THE INVERSE OF THE STOTKNPOSN PROCEDURE.
*/

  IF (HOLDCURPOSN < 0) OR (HOLDTKNBUF.length() EQ 0) OR  ( (unsigned) HOLDCURPOSN > HOLDTKNBUF.length()) THEN
    throw string("out_of_range error from RclTknPosn");
  ENDIF;
  CURPOSN = HOLDCURPOSN;
  TKNBUF  = HOLDTKNBUF;
ENDPROC; // RCLTKNPOSN

// **************************** GETCHR *******************************************
bool FUNCTION PeekChr(CharType REF Char) IS
/*
   THIS IS THE GET CHARACTER ROUTINE.  ITS PURPOSE IS TO GET THE NEXT
   CHARACTER FROM INBUF, DETERMINE ITS FSATYP (FINITE STATE AUTOMATA TYPE),
   AND RETURN THE UPPER CASE VALUE OF CHAR.  RETCOD VALUE OF ONE MEANS AN END OF
   LINE WAS FOUND, TWO MEANS AN ERROR OCCURRED.
   NOTE: THE CURPOSN POINTER IS USED BEFORE IT'S INCREMENTED, UNLIKE MOST OF THE
   OTHER POINTERS IN THIS PROGRAM.
*/
  bool EOL=false;
  IF (unsigned) CURPOSN >= TKNBUF.length() THEN
    EOL = true;
    Char.ch = NullChar;
    Char.STATE = DELIM;
    return EOL;
  ELSE
    EOL = false;
  ENDIF;
  Char.ch = CAP(TKNBUF.at(CURPOSN));    // element access statement
  Char.STATE = FSAARRAY[ (int) Char.ch];       // state assignment, here using array access.
  return EOL;
ENDFUNC; // PeekCHR

PROCEDURE NextChr() IS
  CURPOSN++;
ENDPROC; // NextChr

// ********************************* GetChr ********************************
bool FUNCTION GETCHR(CharType REF Char) IS
  bool EOL;
  EOL = PeekChr(Char);
  NextChr();
  return EOL;
END;// GETCHR

// ********************************* UNGETCHR ********************************
PROCEDURE UNGETCHR() IS
/*
   UNGETCHaracteR.
   THIS IS THE ROUTINE THAT WILL ALLOW THE CHARACTER LAST READ TO BE READ
   AGAIN BY DECREMENTING THE POINTER INTO TKNBUF, CURPOSN.
*/

  IF (CURPOSN < 0) THEN
    throw string("out_of_range error from UnGetChr because CURPOSN < 0");
  ENDIF;
  CURPOSN--;
ENDPROC; // UNGETCHR

// **********************************************************************************
int FUNCTION GETOPCODE(TokenType Token) IS
/*
   GET OPCODE.
   THIS ROUTINE RECEIVES A TOKEN OF FSATYP OP (MEANING IT IS AN OPERATOR)
   AND ANALYZES IT TO DETERMINE AN OPCODE, WHICH IS A Number FROM 1..22.
   THIS IS DONE AFTER THE NECESSARY VALIDITY CHECK OF THE INPUT TOKEN.
   THE OPCODE ASSIGNMENTS FOR THE OP TOKENS ARE:
    < is 1                  <= is 2
    > is 3                  >= is 4
    = is 5   == is 5        <> is 6    # is 7
    + is 8                  += is 9
    - is 10                 -= is 11
    * is 12                 *= is 13
    / is 14                 /= is 15
    ^ is 16                 ^= is 17
   ** is 18                **= is too long to be allowed
   >< is 20
    % is 22
*/
  char CH1,CH2;
  int OpCode=0;

  IF (Token.uStr.length() < 1) OR (Token.uStr.length() > 2) THEN
    throw string("token_too_long error from GetOpCode");
    return OpCode;
  ENDIF;

  CH1 = Token.uStr.at(0);
  IF Token.uStr.length() > 1 THEN
    CH2 = Token.uStr.at(1);
  ELSE
    CH2 = NullChar;
  ENDIF;

  switch (CH1) IS
    case LTSIGN : OpCode = 1; break;
    case GTSIGN : OpCode = 3; break;
    case EQUALSIGN : OpCode = 5; break;
    case  PLUSSIGN  : OpCode = 8; break;
    case  MINUSSIGN : OpCode = 10; break;
    case  POUNDSIGN : OpCode = 7; break;
    case  MULTSIGN  : OpCode = 12; break;
    case  DIVSIGN   : OpCode = 14; break;
    case  EXPSIGN   : OpCode = 16; break;
    case  PERCNT    : OpCode = 22; break;
    default: return OpCode;
  ENDCASE;
  IF Token.uStr.length() EQ 1 THEN
    return OpCode;
  ELSIF (CH2 EQ EQUALSIGN) AND (CH1 NE EQUALSIGN) AND (CH1 NE POUNDSIGN) THEN
    OpCode++;
  ELSIF (CH1 EQ LTSIGN) AND (CH2 EQ GTSIGN) THEN
    OpCode = 6;
  ELSIF (CH1 EQ GTSIGN) AND (CH2 EQ LTSIGN) THEN
    OpCode = 20;
  ELSIF (CH1 EQ MULTSIGN) AND (CH2 EQ MULTSIGN) THEN
    OpCode = 18;
  ELSIF (CH1 EQ EQUALSIGN) AND (CH2 EQ EQUALSIGN) THEN
    // do nothing  
  ELSE // have invalid pair, like +- or =>.  
    UNGETCHR(); // unget the 2nd part of the invalid pair.
  ENDIF; // Length of Token = 1
  return OpCode;
ENDFUNC; // GETOPCODE

//       *************************** GETTKN **************************************
bool FUNCTION GETTKN(TokenType REF TOKEN) IS
  bool NEGATV;
  int ORDCH,upperbound;
  CharType CHAR;
  char QUOCHR;   /* Holds the active quote char */
  bool QUOFLG,EOL;

  QUOCHR = NullChar;
  QUOFLG = false;
  PREVPOSN = CURPOSN;
  TOKEN.STATE = DELIM;
  TOKEN.iSum = 0;
  TOKEN.rSum = 0.0;
  TOKEN.uStr = "";
  NEGATV = false;

  WHILE true DO
    EOL = GETCHR(CHAR);
    IF EOL THEN
//  IF TKNSTATE IS DELIM, THEN GETTKN WAS CALLED WHEN THERE WERE
//  NO MORE TOKENS ON LINE.  OTHERWISE IT MEANS THAT WE HAVE FETCHED THE LAST
//  TOKEN ON THIS LINE.
      IF (TOKEN.STATE EQ DELIM) AND (TOKEN.uStr.length() EQ 0) THEN 
        break;  // with EOL still being true.
      ELSE // now on last token of line, so don't return with EOL set to true.
        EOL = false;
      ENDIF;
    ENDIF; // IF EOL
    ORDCH = CHAR.ch;
    IF QUOFLG AND (CHAR.ch NE NullChar) THEN CHAR.STATE = ALLELSE; ENDIF;
    switch (TOKEN.STATE) IS
      case DELIM : // token.state
        switch (CHAR.STATE) IS
          case DELIM : // NULL char is a special delimiter because it will
                       // immediately cause a return even if there is no token yet,
                       // i.e., the token is only delimiters.  This is because of
                       // the NULL char is the string terminater for general strings
                       // and especially for environment strings, for which this
                       // TOKENIZE module was originally written.
                   IF CHAR.ch EQ NullChar THEN goto ExitLoop; ENDIF;
                   break;
          case OP : 
                     TOKEN.STATE = OP;
                     TOKEN.uStr += CHAR.ch;
                     IF TOKEN.uStr.length() > TKNMAXSIZ THEN
                       throw string("token_too_long in GetToken");
                     ENDIF;
                     break;
          case DGT : 
                     TOKEN.uStr += CHAR.ch;
                     TOKEN.STATE = DGT;
                     TOKEN.iSum = ORDCH - ASCZERO;
                     break;
          case ALLELSE :
                     TOKEN.STATE = ALLELSE;
                     QUOFLG = (CHAR.ch EQ SQUOTE) OR (CHAR.ch EQ DQUOTE);
                     IF QUOFLG THEN     // Do not put the quote character into the token.
                       QUOCHR = CHAR.ch;
                     ELSE
                       TOKEN.uStr += CHAR.ch;
                       IF TOKEN.uStr.length() > TKNMAXSIZ THEN
                         throw string("token_too_long in GetToken");
                       ENDIF; // if token too long
                       TOKEN.iSum = ORDCH;
                     ENDIF; // QUOFLG
        ENDCASE; // Of Char.State
        break; 
      case OP : // token.state
        switch (CHAR.STATE) IS
          case DELIM : 
            UNGETCHR(); // To allow correct processing of op pair that is not a valid op, like +- or =>
            goto ExitLoop;
            break;
          case OP :
            IF TOKEN.uStr.length() > OPMAXSIZ THEN
              UNGETCHR();
              goto ExitLoop;
            ENDIF;
            TOKEN.uStr += CHAR.ch;
            break;
          case DGT :
            IF (TOKEN.uStr[TOKEN.uStr.length()-1] EQ '+') OR (TOKEN.uStr[TOKEN.uStr.length()-1] EQ '-') THEN
              IF TOKEN.uStr.length() EQ 1 THEN
                IF TOKEN.uStr[0] EQ '-' THEN NEGATV = true; ENDIF;
                TOKEN.STATE = DGT;
                TOKEN.uStr = ""; // OVERWRITE ARITHMETIC SIGN CHARACTER
                TOKEN.uStr += CHAR.ch;
                TOKEN.iSum = ORDCH - ASCZERO;
              ELSE  // TOKEN length > 1 so must first return valid OP
                UNGETCHR(); /* UNGET THIS DIGIT CHAR */
                UNGETCHR(); /* THEN UNGET THE ARITH SIGN CHAR */
                upperbound = TOKEN.uStr.length() - 1;
// uses -std=c++11                CHAR.ch = TOKEN.uStr.back(); // SO DELIMCH CORRECTLY RETURNS THE ARITH SIGN CHAR
                CHAR.ch = TOKEN.uStr.at(upperbound); // SO DELIMCH CORRECTLY RETURNS THE ARITH SIGN CHAR
                TOKEN.uStr = TOKEN.uStr.erase(TOKEN.uStr.length()-1); // del last char of the token which is the sign character
                goto ExitLoop;
              ENDIF; // if length of the token = 1
            ELSE // IF have a sign character
              UNGETCHR();
              goto ExitLoop;
            ENDIF; // If have a sign character
            break;
          case ALLELSE :
            UNGETCHR();
            goto ExitLoop;
            break;
        ENDCASE; // Char.State
        break;
      case DGT : // tokenstate
        switch (CHAR.STATE) IS
          case DELIM : goto ExitLoop;
          case OP :
                    UNGETCHR();
                    goto ExitLoop;
          case DGT :
                    TOKEN.uStr += CHAR.ch;
                    TOKEN.iSum = 10*TOKEN.iSum + ORDCH - ASCZERO;
                    break;
          case ALLELSE : 
                    UNGETCHR();
                    goto ExitLoop;
        ENDCASE; // Char.State
        break;
      case ALLELSE : // tokenstate
        switch (CHAR.STATE) IS
          case DELIM :
            //  Always exit if get a NULL char as a delim.  A quoted string can only get here if CH is NULL.
                    goto ExitLoop;
          case OP :
                    UNGETCHR();
                    goto ExitLoop;
          case DGT :
                     IF TOKEN.uStr.length() > TKNMAXSIZ THEN
                       throw string("token_too_long in GetTkn AllElse branch.");
                     ENDIF; // if token too long
                     TOKEN.uStr += CHAR.ch;
                     TOKEN.iSum += ORDCH;
                     break;
          case ALLELSE :
                     IF CHAR.ch EQ QUOCHR THEN
                       QUOFLG = false;
                       CHAR.STATE = DELIM; // So that DELIMSTATE will = delim
                       goto ExitLoop;
                     ELSE
                       IF TOKEN.uStr.length() > TKNMAXSIZ THEN
                         throw string("token_too_long in GetTkn AllElse branch");
                       ENDIF; // if token too long
                       TOKEN.uStr += CHAR.ch;
                       TOKEN.iSum += ORDCH;
                     ENDIF; // if char is a quote char
                     break;
        ENDCASE; // Char.State
    ENDCASE; // Token.State
  ENDWHILE; //LOOP to process characters

  ExitLoop:
  TOKEN.delimCH    = CHAR.ch;
  TOKEN.DelimState = CHAR.STATE;
  IF (TOKEN.STATE EQ DGT) AND NEGATV THEN TOKEN.iSum = -TOKEN.iSum; ENDIF;
//  For OP tokens, must return the opcode as the sum value.  Do this by calling GETOPCODE.
  IF TOKEN.STATE EQ OP THEN
    TOKEN.iSum = GETOPCODE(TOKEN);
  ENDIF;
  TOKEN.rSum = TOKEN.iSum;
  return EOL;
ENDFUNC; // GETTKN
//***********************************************************************************
bool FUNCTION ishexdigit(char ch) IS

  bool ishex;

  ishex = isdigit(ch) OR ((ch >= 'A') AND (ch <= 'F'));
  return ishex;

ENDFUNC; // ishexdigit
//************************************************************************************
int FUNCTION FromHex(string s) IS
  int result = 0;
  int dgtval = 0; // compiler flagged this line w/ a warning that this variable may be uninitialized by its use below.  Fixed 10 Feb 2020.
  char dgtchar;
  const int OrdinalZero = '0';
  const int OrdinalCapA = 'A';
  string::iterator t;

  FOR t = s.begin(); t NE s.end(); t++ DO 
    dgtchar = *t;
    IF isdigit(dgtchar) THEN
      dgtval = dgtchar - OrdinalZero;
    ELSIF ishexdigit(dgtchar) THEN
      dgtval = dgtchar - OrdinalCapA + 10;
    ENDIF;  // ignore blanks or any other non digit character.  This includes ignoring the trailing 'H'.
    result = 16*result + dgtval;
  ENDFOR;
  return result;
ENDFUNC; // FromHex


//******************************************** GETTKNREAL ***************************************
// I am copying the working code from TKNRTNS here.  See the comments in tknrtnsa.adb for reason why.
bool FUNCTION GETTKNREAL(TokenType REF TOKEN) IS
  bool EOL;
  CharType CHAR;
  int Len;
//  char ch;

  TOKEN.STATE = DELIM;
  TOKEN.iSum = 0;
  TOKEN.rSum = 0.0;
  TOKEN.uStr = "";


  EOL = GETTKN(TOKEN);

  Len = TOKEN.uStr.length();
//    uses -std=c++11                       IF (TOKEN.STATE EQ ALLELSE) AND (Len > 1) AND (TOKEN.uStr.front() EQ '.') AND isdigit(TOKEN.uStr.at(1)) THEN
  IF (TOKEN.STATE EQ ALLELSE) AND (Len > 1) AND (TOKEN.uStr.at(0) EQ '.') AND isdigit(TOKEN.uStr.at(1)) THEN
// Likely have a real number beginning with a decimal point, so fall thru to the digit token
  ELSIF TOKEN.STATE NE DGT THEN
    return EOL;
  ENDIF;
//
// Now must have a digit token.  
// 
  UNGETTKN();
  TOKEN.uStr = "";
  TOKEN.STATE = DGT;
  PREVPOSN = CURPOSN;

  WHILE true DO
    EOL = GETCHR(CHAR);
    IF EOL THEN
//                                     IF TKNSTATE IS DELIM, THEN GETTKN WAS CALLED WHEN THERE WERE
//                                     NO MORE TOKENS ON LINE.  OTHERWISE IT MEANS THAT WE HAVE FETCHED THE LAST
//                                     TOKEN ON THIS LINE.
      IF (TOKEN.STATE EQ DELIM) AND (TOKEN.uStr.length() EQ 0) THEN 
        break;  // with EOL still being true.
      ELSE // now on last token of line, so don't return with EOL set to true.
        EOL = false;
      ENDIF; // if token state is a delim and token str is empty
    ENDIF; // IF EOL

    switch (CHAR.STATE) IS
      case DELIM :                    // Ignore leading delims
                   IF TOKEN.uStr.length() > 0 THEN goto ExitLoop; ENDIF;
                   break;
      case OP :
                Len = TOKEN.uStr.length();
                IF ((CHAR.ch NE '+') AND (CHAR.ch NE '-')) OR 
                              ((TOKEN.uStr.length() > 0) AND (TOKEN.uStr.at(Len-1) NE 'E')) THEN
// uses -std=c++11                              ((TOKEN.uStr.length() > 1) AND (TOKEN.uStr.back() NE 'E')) THEN
                  UNGETCHR();
                  goto ExitLoop;
                ENDIF;
                TOKEN.uStr += CHAR.ch;
                break;
      case DGT :
                TOKEN.uStr += CHAR.ch;
                break;
      case ALLELSE :
               IF (CHAR.ch NE '.') AND (CHAR.ch NE 'E') AND NOT ishexdigit(CHAR.ch) AND (CHAR.ch NE 'H') THEN
                 UNGETCHR();
                 goto ExitLoop;
               ENDIF;
               TOKEN.uStr += CHAR.ch;
    ENDCASE; // Char State
  ENDWHILE; // getting characters loop

  ExitLoop: 

  TOKEN.delimCH    = CHAR.ch;
  TOKEN.DelimState = CHAR.STATE;
  Len = TOKEN.uStr.length();
//                                                          cout << "uStr=" << TOKEN.uStr << ", len=" << Len << endl;
// uses -std=c++11                                          IF TOKEN.uStr.back() EQ 'H' THEN
  IF TOKEN.uStr.at(Len-1) EQ 'H' THEN
    TOKEN.uStr = TOKEN.uStr.erase(Len-1); //  must remove the 'H' from uStr.
    TOKEN.iSum = FromHex(TOKEN.uStr);
    TOKEN.rSum = TOKEN.iSum;
  ELSE
    IF TOKEN.uStr.at(0) EQ '.' THEN
      TOKEN.uStr.insert(0,1,'0');   // insert leading 0 if number begins with a decimal point.
    ENDIF;
    TOKEN.rSum = stod(TOKEN.uStr);
    TOKEN.iSum = (int) TOKEN.rSum;
  ENDIF;
/*
  I want EOL to only return TRUE when there is no token to process, so I have to make sure that EOL
  is set to false if there is a token here what was processed.
*/
  IF EOL and TOKEN.STATE NE DELIM THEN
    EOL = false;
  ENDIF;
  
  return EOL;
ENDFUNC; // GETTKNREAL
 



// ******************************************************************************

bool FUNCTION GETTKNSTR(TokenType REF TOKEN) IS
  int c;
  bool EOL;
  CharType Char;
  FOR c=Dgt0; c<=Dgt9; c++ DO
    FSAARRAY[c] = ALLELSE;
  ENDFOR;
  FSAARRAY['#'] = ALLELSE; /* poundsign */
  FSAARRAY['*'] = ALLELSE; /* multsign */
  FSAARRAY['+'] = ALLELSE; /* plussign */
  FSAARRAY['-'] = ALLELSE; /* minussign */
  FSAARRAY['/'] = ALLELSE; /* divsign */
  FSAARRAY['<'] = ALLELSE; /* LTSIGN */
  FSAARRAY['='] = ALLELSE; /* EQUAL */
  FSAARRAY['>'] = ALLELSE; /* GTSIGN */

  EOL = GETTKN(TOKEN);
  IF EOL OR (TOKEN.STATE EQ DELIM) OR ((TOKEN.STATE EQ ALLELSE) AND (TOKEN.DelimState EQ DELIM)) THEN
    return EOL;
  ENDIF;

/*
  Now must do special function of this proc.
  Continue building the string as left off from GETTKN call.
  As of 6/95 this should always return a tknstate of allelse, so return.
*/
  WHILE true DO
    EOL = GETCHR(Char);
    IF EOL OR ((Char.STATE EQ DELIM) AND (TOKEN.uStr.length() > 0)) THEN
      break; // Ignore leading delims
    ENDIF;
    TOKEN.uStr += Char.ch;
    TOKEN.iSum += Char.ch;
  ENDWHILE; // getting chars
  TOKEN.rSum = TOKEN.iSum;
  TOKEN.delimCH = Char.ch;
  TOKEN.DelimState = Char.STATE;
  IF EOL and TOKEN.STATE NE DELIM THEN
    EOL = false;
  ENDIF;
  return EOL;
ENDFUNC;  // GetTknStr
// ***********************************************************************************
bool FUNCTION GETTKNEOL(TokenType REF TOKEN) IS
// GET ToKeN to EndOfLine.
// This will build a token that consists of every character left on the line.
// That is, it only stops at the end of line.
// The TRIM procedure is used to set the COUNT and LENGTH fields.  This is
// the only TOKENIZE procedure that uses it.

  CharType Char;
  bool EOL;

  PREVPOSN = CURPOSN; /* So this tkn can be ungotten as well */
  TOKEN.STATE = DELIM;
  TOKEN.iSum = 0;
  TOKEN.rSum = 0.0;
  TOKEN.uStr = "";
  WHILE true DO
    EOL = GETCHR(Char);
    IF EOL THEN break; ENDIF;
    TOKEN.uStr += Char.ch;
    TOKEN.iSum += Char.ch;
    TOKEN.STATE = ALLELSE;
  ENDWHILE;
  TOKEN.rSum = TOKEN.iSum;
  TOKEN.delimCH = NullChar;
  TOKEN.DelimState = DELIM;
  IF EOL and TOKEN.STATE NE DELIM THEN 
    EOL = false;
  ENDIF;
  return EOL;
ENDFUNC;// GETTKNEOL

//************************************** UNGETTKN *****************************
PROCEDURE UNGETTKN() IS
/*
 * UNGET TOKEN ROUTINE.
 This routine will unget the last token fetched.  It does this by restoring
 the previous value of POSN, held in PREVPOSN.  Only the last token fetched
 can be ungotten, so PREVPOSN is reset after use.  If PREVPOSN contains this
 as its value, then the unget operation will fail.
*/
  IF (CURPOSN <= PREVPOSN) OR (PREVPOSN < 0) THEN
    throw string("out_of_range in UnGetTkn");
  ENDIF;  // End error trap

  CURPOSN  = PREVPOSN;
  PREVPOSN = 0;
ENDPROC;


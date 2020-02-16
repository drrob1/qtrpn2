#include "macros.h"

  enum FSATYP {DELIM, OP, DGT, ALLELSE};
  struct TokenType {
    string uStr;
    FSATYP STATE;
    char delimCH;
    FSATYP DelimState;
    int iSum;    // IntSum
    double rSum; // RealSum
  }; // TokenType record

  struct CharType { 
    char ch;
    FSATYP STATE;
  }; // CharType Record

// ---------------------------------------------------------------------
//
char FUNCTION Cap(char c);

PROCEDURE INITKN(string Str);

PROCEDURE STOTKNPOSN();

PROCEDURE RCLTKNPOSN();

bool FUNCTION PeekChr(CharType REF Char);
PROCEDURE NextChr();

bool FUNCTION GETCHR(CharType REF Char);

PROCEDURE UNGETCHR();

bool FUNCTION GETTKN(TokenType REF TOKEN);

bool FUNCTION GETTKNREAL(TokenType REF TOKEN);

bool FUNCTION GETTKNSTR(TokenType REF TOKEN);

bool FUNCTION GETTKNEOL(TokenType REF TOKEN);

PROCEDURE UNGETTKN();

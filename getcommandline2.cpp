/*
 *  Created on: Oct 27, 2014
 *      Author: rob
 */
#include <cstring>
#include <cstdio>
#include <iostream>
using namespace std;
#include <cstdlib>
#include <cctype>
#include <cmath>
#include <ctime>
#include <string>

#include "getcommandline2.h"

//#include "macros.h"

/*
#define EQ ==
#define NE !=
#define BEGIN {
#define END }
#define ENDIF }
#define ENDFOR }
#define ENDCASE }
#define ENDLOOP }
#define MOD %
#define AND &&
#define OR ||
#define NOT !
#define THEN {
#define ELSE }else{
#define ELSIF }else if{
#define IS {
#define DO {
#define LOOP {
#define ADROF &
#define ADR &
#define DEREF *
#define POINTERTO *
#define VAR &
#define REF &
#define PROCEDURE void
#define FUNCTION

typedef FILE* FileType;
*/

FUNCTION string getcommandline(int argc, char *argv[]) IS
//		string s="";
		string s;
                int i;
		if (argc <= 1) {
		  return (s);
        };
        s = argv[1];
        for (i=2; i < argc; ++i) {
                s = s.append(" ");
 		        s = s.append(argv[i]);
        };
 		return s;
END;


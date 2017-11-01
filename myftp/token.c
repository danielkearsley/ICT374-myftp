// file:		token.c for Week 7
// author:		HX
// date:		2006.09.14
// last modified:	2006.09.21
// note:		not thoroughlt tested, therefore it may contain errors

#include <string.h>
#include "token.h"

int tokenise (char line[], char *token[])
{
      char *tk;
      int i=0;

      tk = strtok(line, tokenSeparators);
      token[i] = tk;

      while (tk != NULL) {

          ++i;
          if (i>=MAX_NUM_TOKENS) {
              i = -1;
              break;
          }

          tk = strtok(NULL, tokenSeparators);
          token[i] = tk;
      }
      return i;
}


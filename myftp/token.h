/**
 * AUTHOR: Clem Davies, Daniel Kearsley
 * DATE: 3/11/17
 * FILENAME: token.c
 * DESCRIPTION: Defines a function to split a string on seperators.
 */

#define MAX_NUM_TOKENS  100
#define tokenSeparators " \t\n"    // characters that separate tokens

/*
 * breaks up an array of chars by whitespace characters into individual tokens.
 * return
 * >=0: largest index of token array
 * -1: on failure
 */
int tokenise (char line[], char *token[]);


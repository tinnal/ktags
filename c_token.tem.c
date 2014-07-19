#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "c_token.h"


MyToken* NewToken(char *file, long startLine, long startCol,
	long endLine, long endCol, char* tokenStart, char* tokenEnd, int type)
{
	MyToken *token;
	int tokenLen = tokenEnd - tokenStart;
	char *tokenName = NULL;

	if((token = (MyToken*)malloc(sizeof(MyToken))) == NULL)
		return NULL;
	if( tokenLen == 0 || (tokenName = (char*)malloc(tokenLen + 1)) == NULL)
	{
		free(token);
		return NULL;
	}

	memset(token, 0, sizeof(MyToken));
	strncpy(tokenName, tokenStart, tokenLen);
	tokenName[tokenLen] = 0;
	
	token->file = file;
	token->startLine = startLine;
	token->startCol = startCol;
	token->endLine = endLine;
	token->endCol = endCol;
	token->name = tokenName;
	token->tokenType = type;

//	printf("Add token %s, %d:%d - %d:%d\n", token->name, startLine, startCol, endLine, endCol);

	return token;
}

/*
PushToken(TokenNode* tokenStack, MyToken* token)
{
}

MyToken* PopToken(TokenNode* tokenStack)
{
}*/

TokenNode* AddToken(TokenNode* tokenList,  MyToken* token)
{
	if(token == NULL || tokenList == NULL)
		return NULL;
	TokenNode *tokenNode = NULL;
	if((tokenNode = (TokenNode*)malloc(sizeof(TokenNode))) == NULL)
		return NULL;
	memset(tokenNode, 0, sizeof(TokenNode));
	tokenNode->mime = token;
	tokenNode->next = tokenList;
	//scope->tokenList.prev point to last tokenNode
	tokenNode->prev = tokenList->prev;
	tokenList->prev->next = tokenNode;
	tokenList->prev = tokenNode;
	return tokenNode;
}

TokenNode* DelTokenNode(TokenNode* delTokenNode)
{
	TokenNode *nextNode;
	printf("del token %s\n",delTokenNode->mime->name);
	delTokenNode->prev->next = delTokenNode->next;
	delTokenNode->next->prev = delTokenNode->prev;
	nextNode = delTokenNode->prev;
	free(delTokenNode);
	return nextNode;
}


TokenNode* InsertToken(TokenNode* insertPoint,  MyToken* token)
{
	if(token == NULL || insertPoint == NULL)
		return NULL;
	TokenNode *newTokenNode = NULL;
	printf("intert token %s\n",token->name);
	if((newTokenNode = (TokenNode*)malloc(sizeof(TokenNode))) == NULL)
		return NULL;
	memset(newTokenNode, 0, sizeof(TokenNode));
	newTokenNode->mime = token;

	newTokenNode->next = insertPoint->next;
	newTokenNode->prev = insertPoint;
	insertPoint->next->prev = newTokenNode;
	insertPoint->next = newTokenNode;

	return newTokenNode;
}





MyToken* NextToken(TokenNode** nextNode)
{
retry:
	*nextNode = (*nextNode)->next;
	if((*nextNode)->mime->tokenType == EOFILE)
		return NULL;
	if((*nextNode)->mime->tokenType == SPACE)
		goto retry;
	return (*nextNode)->mime;
}


#define	YYCTYPE		char
#define	YYCURSOR	yyCursor
#define	YYLIMIT		yyLimit
#define	YYMARKER	yyMarker
#define	YYFILL(n)	
#define ADD_TOKEN(X)  	{\
	endLine = currLine;	\
	endCol = (int)(yyCursor - lineStart) -1; \
	AddToken(tokenList, \
	NewToken(fileName, startLine, startCol, endLine, endCol, yyToken, yyCursor,(X))); \
	}



int OpenTokenFile(char *fileName, TokenNode *tokenList){
	char *buffer, *lineStart;
	char *yyMarker, *yyLimit, *yyCursor, *yyToken;
	int currLine = 0, startLine = 0, startCol = 0, endLine = 0, endCol = 0;
	int fileSize, readCount;
	FILE* tokenFd;
	int currPos;

	tokenFd = fopen(fileName, "rb");
	if(tokenFd == NULL)
		return -1;
	fseek(tokenFd, 0, SEEK_END);
	fileSize = ftell(tokenFd);
	fseek(tokenFd, 0, SEEK_SET);
	buffer = (char*)malloc(fileSize+1);
	if(buffer == NULL) return -1;
	lineStart = yyCursor = yyToken = yyMarker = buffer;
	yyLimit = buffer + fileSize;

	readCount= 0;
	while(readCount != fileSize)
	{
		readCount += fread(buffer+readCount, 1, 4096, tokenFd);
	}
	buffer[readCount] = 0;

	fclose(tokenFd);
	
	for(;;)
	{
		yyToken = yyCursor;
		startLine = currLine;
		startCol = (int)(yyToken - lineStart);
			
		/*!re2c
		any	= [\000-\377];
		O	= [0-7];
		D	= [0-9];
		L	= [a-zA-Z_];
		H	= [a-fA-F0-9];
		E	= [Ee] [+-]? D+;
		FS	= [fFlL];
		IS	= [uUlL]*;
		ESC	= [\\] ([abfnrtv?'"\\] | "x" H+ | O+);
		*/

		/*!re2c
		"/*"			{ goto comment; }
		"//"			{ goto cppcomment;}
		
		"auto"		{ ADD_TOKEN(AUTO); continue;}
		"break"		{ ADD_TOKEN(BREAK); continue;}
		"case"		{ ADD_TOKEN(CASE); continue;}
		"char"		{ ADD_TOKEN(CHAR); continue;}
		"const"		{ ADD_TOKEN(CONST); continue;}
		"continue"	{ ADD_TOKEN(CONTINUE); continue;}
		"default"		{ ADD_TOKEN(DEFAULT); continue;}
		"do"			{ ADD_TOKEN(DO); continue;}
		"double"		{ ADD_TOKEN(DOUBLE); continue;}
		"else"		{ ADD_TOKEN(ELSE); continue;}
		"enum"		{ ADD_TOKEN(ENUM); continue;}
		"extern"		{ ADD_TOKEN(EXTERN); continue;}
		"float"		{ ADD_TOKEN(FLOAT); continue;}
		"for"			{ ADD_TOKEN(FOR); continue;}
		"goto"		{ ADD_TOKEN(GOTO); continue;}
		"if"			{ ADD_TOKEN(IF); continue;}
		"int"			{ ADD_TOKEN(INT); continue;}
		"long"		{ ADD_TOKEN(LONG); continue;}
		"register"		{ ADD_TOKEN(REGISTER); continue;}
		"return"		{ ADD_TOKEN(RETURN); continue;}
		"short"		{ ADD_TOKEN(SHORT); continue;}
		"signed"		{ ADD_TOKEN(SIGNED); continue;}
		"sizeof"		{ ADD_TOKEN(SIZEOF); continue;}
		"static"		{ ADD_TOKEN(STATIC); continue;}
		"struct"		{ ADD_TOKEN(STRUCT); continue;}
		"switch"		{ ADD_TOKEN(SWITCH); continue;}
		"typedef"		{ ADD_TOKEN(TYPEDEF); continue;}
		"union"		{ ADD_TOKEN(UNION); continue;}
		"unsigned"	{ ADD_TOKEN(UNSIGNED); continue;}
		"void"		{ ADD_TOKEN(VOID); continue;}
		"volatile"		{ ADD_TOKEN(VOLATILE); continue;}
		"while"		{ ADD_TOKEN(WHILE); continue;}
		"__attribute__"	{ ADD_TOKEN(ATTRIBUTE); continue;}
		"inline"		{ ADD_TOKEN(INLINE); continue;}
		"__restrict__"		{ADD_TOKEN(RESTRICT); continue;}
		"__inline__"	{ ADD_TOKEN(INLINE); continue;}


		"#"[ \t\v\f]*"if"			{ ADD_TOKEN(PP_IF); continue;}
		"#"[ \t\v\f]*"else"			{ ADD_TOKEN(PP_ELSE); continue;}
		"#"[ \t\v\f]*"ifdef"			{ ADD_TOKEN(PP_IFDEF); continue;}
		"#"[ \t\v\f]*"endif"			{ ADD_TOKEN(PP_ENDIF); continue;}
		"#"[ \t\v\f]*"include"		{ ADD_TOKEN(PP_INCLUDE); continue;}
		"#"[ \t\v\f]*"define"		{ ADD_TOKEN(PP_DEFINE); continue;}
		"#"[ \t\v\f]*"undef"		{ ADD_TOKEN(PP_UNDEF); continue;}
		"#"[ \t\v\f]*"line"			{ ADD_TOKEN(PP_LINE); continue;}
		"#"[ \t\v\f]*"error"			{ ADD_TOKEN(PP_ERROR); continue;}
		"#"[ \t\v\f]*"pragma"		{ ADD_TOKEN(PP_PRAGMA); continue;}
		"#"[ \t\v\f]+* 			{ ADD_TOKEN(PP_SHARP); continue;}
		[ \t\v\f]*	"##"[ \t\v\f]+*  	{ ADD_TOKEN(PP_DSHARP); continue;}
		"defind"					{ ADD_TOKEN(PP_DEFINED); continue;}
		"__LINE__"				{ ADD_TOKEN(PP_LINENO); continue;}
		"__FILE__"				{ ADD_TOKEN(PP_FILE); continue;}
		"__DATE__"				{ ADD_TOKEN(PP_DATE); continue;}
		"__STDC__"				{ ADD_TOKEN(PP_STDC); continue;}
		"eval"					{ ADD_TOKEN(PP_EVAL); continue;}



		L (L|D)*		{ ADD_TOKEN(ID); continue;}
		
		("0" [xX] H+ IS?) | ("0" D+ IS?) | (D+ IS?) |
		(['] (ESC|any\[\n\\'])* ['])
					{ ADD_TOKEN(ICON); continue;}
		
		(D+ E FS?) | (D* "." D+ E? FS?) | (D+ "." D* E? FS?)
					{ ADD_TOKEN(FCON); continue;}
		
		(["] (ESC|any\[\n\\"])* ["])
					{ ADD_TOKEN(SCON); continue;}
		
		"..."              { ADD_TOKEN(ELLIPSIS); continue;}
		">>="		{ ADD_TOKEN(RSHIFTEQ); continue;}
		"<<="		{ ADD_TOKEN(LSHIFTEQ); continue;}
		"+="			{ ADD_TOKEN(ADDEQ); continue;}
		"-="			{ ADD_TOKEN(SUBEQ); continue;}
		"*="			{ ADD_TOKEN(MULEQ); continue;}
		"/="			{ ADD_TOKEN(DIVEQ); continue;}
		"%="		{ ADD_TOKEN(MODEQ); continue;}
		"&="			{ ADD_TOKEN(ANDEQ); continue;}
		"^="			{ ADD_TOKEN(XOREQ); continue;}
		"|="			{ ADD_TOKEN(OREQ); continue;}
		">>"			{ ADD_TOKEN(RSHIFT); continue;}
		"<<"			{ ADD_TOKEN(LSHIFT); continue;}
		"++"			{ ADD_TOKEN(INCR); continue;}
		"--"			{ ADD_TOKEN(DECR); continue;}
		"->"			{ ADD_TOKEN(DEREF); continue;}
		"&&"			{ ADD_TOKEN(ANDAND); continue;}
		"||"			{ ADD_TOKEN(OROR); continue;}
		"<="			{ ADD_TOKEN(LEQ); continue;}
		">="			{ ADD_TOKEN(GEQ); continue;}
		"=="			{ ADD_TOKEN(EQL); continue;}
		"!="			{ ADD_TOKEN(NEQ); continue;}
		";"			{ ADD_TOKEN(';'); continue;}
		"{"			{ ADD_TOKEN('{'); continue;}
		"}"			{ ADD_TOKEN('}'); continue;}
		","			{ ADD_TOKEN(','); continue;}
		":"			{ ADD_TOKEN(':'); continue;}
		"="			{ ADD_TOKEN('='); continue;}
		"("			{ ADD_TOKEN('('); continue;}
		")"			{ ADD_TOKEN(')'); continue;}
		"["			{ ADD_TOKEN('['); continue;}
		"]"			{ ADD_TOKEN(']'); continue;}
		"."			{ ADD_TOKEN('.'); continue;}
		"&"			{ ADD_TOKEN('&'); continue;}
		"!"			{ ADD_TOKEN('!'); continue;}
		"~"			{ ADD_TOKEN('~'); continue;}
		"-"			{ ADD_TOKEN('-'); continue;}
		"+"			{ ADD_TOKEN('+'); continue;}
		"*"			{ ADD_TOKEN('*'); continue;}
		"/"			{ ADD_TOKEN('/'); continue;}
		"%"			{ ADD_TOKEN('%'); continue;}
		"<"			{ ADD_TOKEN('<'); continue;}
		">"			{ ADD_TOKEN('>'); continue;}
		"^"			{ ADD_TOKEN('^'); continue;}
		"|"			{ ADD_TOKEN('|'); continue;}
		"?"			{ ADD_TOKEN('?'); continue;}

		[ \t\v\f]+		{ ADD_TOKEN(SPACE); continue;}
		"\r"?"\n"
		{
			ADD_TOKEN(NEWLINE);
			lineStart = yyCursor;
			currLine++;
			continue;
		}
		"\000"		{ADD_TOKEN(EOFILE); free(buffer); return 0; }
		any
		{
			printf("unexpected character: %c\n", yyToken);
			continue;
		}
		*/
		comment:
		/*!re2c
		"*""/"			{ ADD_TOKEN(COMMENT); continue; }
		"\r"?"\n"
		{
			lineStart = yyCursor;
			currLine++;
			goto comment;
		}
		any			{ goto comment; }
		*/
		cppcomment:
		/*!re2c
		"\r"?"\n"		
		{
			ADD_TOKEN(COMMENT);
			lineStart = yyCursor;
			currLine++;
			continue;
		}
		any			{ goto cppcomment; }
		*/
	}
}


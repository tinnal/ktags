
#ifdef HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include "c_token.h"
#include "c_parser.h"
#include "c_pp.h"

#define error printf

#define	NLSIZE	128
static Macro* gMacroList[NLSIZE] = {0};
Macro* LockupMacro(MyToken* token, int* exist, int install)
{
	Macro *macro;
	unsigned char *cp, hash;
	hash = 0;
	for (cp = token->name; *cp != 0; cp++)
		hash += *cp;
	hash %= NLSIZE;
	macro = gMacroList[hash];
	while (macro) {
		if (strcmp(macro->macroName->name, token->name)==0)
		{
			*exist = 1;
			return macro;
		}
		macro = macro->next;
	}
	if (install) {
		macro = (Macro*)malloc(sizeof(Macro));
		memset(macro, 0, sizeof(Macro));
		macro->args.next = macro->args.prev = &(macro->args);
		macro->body.next = macro->body.prev = &(macro->body);
		macro->macroName =  token;
		macro->next = gMacroList[hash];
		gMacroList[hash] = macro;
		
		return macro;
	}
	return NULL;
}


dodefine(TokenNode **currNode)
{
	TokenNode /**nextNode,*/ *argNode;
	MyToken *token;
	Macro *macro;
	int exist = 0;

	token = NextToken(currNode);
	if (token == NULL || token->tokenType!=ID) {
		error("#defined token is not a name");
		return;
	}

	macro = LockupMacro(token, &exist, 1);
	if (exist) {
		error("#defined token %s can't be redefined", token->name);
		return;
	}
	/* collect arguments */
	token = NextToken(currNode);
	if (token && token->tokenType=='(') {
		/* macro with args */
		int narg = 0;
		token = NextToken(currNode);
		if (token->tokenType!=')') {
			int err = 0;
			for (;;) {
				if (token->tokenType!=ID) {
					err++;
					break;
				}
				for (argNode=macro->args.next; argNode != &macro->args; 
					argNode =argNode->next ){
					if (strcmp((char*)argNode->mime->name, token->name)==0)
						error("Duplicate macro argument");
				}
				AddToken(&macro->args, token);
				narg++;
				token = NextToken(currNode);
				macro->argsCount = narg;
				if (token->tokenType==')')
					break;
				if (token->tokenType!=',') {
					err++;
					break;
				}
				token = NextToken(currNode);
			}
			if (err) {
				error("Syntax error in macro parameters");
				return;
			}
		}
		token = NextToken(currNode);
	}

	/* collect macro body */
	for (; (*currNode)->mime->tokenType!= NEWLINE && (*currNode)->mime->tokenType!= EOFILE; 
		(*currNode) = (*currNode)->next )
	{
		int argNo = 0;
		for (argNode=macro->args.next; argNode != &macro->args; 
			argNode =argNode->next ){
			if (strcmp((char*)argNode->mime->name, (*currNode)->mime->name)==0){
				(*currNode)->mime->ppDef = argNode->mime;
				(*currNode)->mime->ppArgNo = argNo;
			}
			argNo++;
		}		
		AddToken(&macro->body, (*currNode)->mime);
	}
}


inline MyToken* InstantiateArg(MyToken **actualArgs, MyToken *token)
{
	if(token->ppDef != NULL)
		//token is format arg, translate it into actual argument.
		return actualArgs[token->ppArgNo];
	else
		return token;	
}

TokenNode* expand(TokenNode *tokenNode, Macro *macro)
{
	TokenNode *beganTokenNode = tokenNode;
	TokenNode *currNode = tokenNode;
	TokenNode *bodyNode, *insertPoint, *delNode, *saveNode, *ret;
	MyToken *token, **ArgsArray, *token1, *token2;
	MyToken *currToken, *newToken;
	
	// collect arguments 
	token = NextToken(&currNode);
	if (token && token->tokenType=='(') {
		/* macro with args */
		int narg = 0;
		ArgsArray = (MyToken**)malloc(sizeof(MyToken*)*(macro->argsCount));
		token = NextToken(&currNode);
		if (token->tokenType!=')') {
			int err = 0;
			for (;;) {
				if (token->tokenType!=ID) {
					err++;
					break;
				}
				ArgsArray[narg] = token;
				if (++narg > macro->argsCount){
					err++;
					break;
				}

				token = NextToken(&currNode);
				if (token->tokenType==')')
					break;
				if (token->tokenType!=',') {
					err++;
					break;
				}
				token = NextToken(&currNode);
			}

			if (err) {
				free(ArgsArray);
				error("Syntax error in macro parameters");
				return NULL;
			}

		}
		token = NextToken(&currNode);
	}

	//del the original tokens
	for (delNode=beganTokenNode; delNode != currNode; delNode = delNode->next){
		delNode = DelTokenNode(delNode);
	}	

	//insert the expanded token
	//ret = appendNode = AddToken(beganTokenNode, token);
	insertPoint = currNode->prev;
	ret = beganTokenNode->prev;
	for (bodyNode=macro->body.next; bodyNode != &macro->body; 
		bodyNode =bodyNode->next )
	{
		switch(bodyNode->mime->tokenType)
		{
		case ID:
			insertPoint = InsertToken(insertPoint, InstantiateArg(ArgsArray,bodyNode->mime));
			break;
		case PP_DSHARP:
			//Pop the prev token we had processed.
			token1 = insertPoint->mime;
			insertPoint = DelTokenNode(insertPoint);
			token2 = NextToken(&bodyNode);//TODO: ERROR
			token2 = InstantiateArg(ArgsArray,token2);
			newToken = (MyToken*)malloc(sizeof(MyToken));
			memset(newToken, 0 , sizeof(MyToken));
			newToken->genToken1 = bodyNode->prev->mime;
			newToken->genToken2 = token1;
			newToken->tokenType = ID;
			newToken->name = (char*)malloc(strlen(token1->name)+strlen(token2->name)+1);
			sprintf(newToken->name, "%s%s", token1->name, token2->name);
			insertPoint = InsertToken(insertPoint, newToken);
			break;
		case PP_SHARP:
			token1 = NextToken(&bodyNode); //TODO: ERROR
			token1 = InstantiateArg(ArgsArray,token1);
			newToken = (MyToken*)malloc(sizeof(MyToken));
			memset(newToken, 0 , sizeof(MyToken));
			newToken->genToken1 = bodyNode->mime;
			newToken->tokenType = SCON;
			newToken->name = (char*)malloc(strlen(token1->name)+3);
			sprintf(newToken->name, "\"%s\"", token1->name, token1->name);
			insertPoint = InsertToken(insertPoint, newToken);
			break;
		default:
			insertPoint = InsertToken(insertPoint, bodyNode->mime);
			break;
		}			
	}
	
	return ret;
}


void
C_PP(TokenNode *tokenList)
{
	MyToken *currToken, *nextToken, *funcToken =NULL;
	TokenNode *currNode = tokenList, *nextNode;
	Macro *macro;
	int exist;

	for(currNode = currNode->next;
		currNode->mime->tokenType != EOFILE ;
		currNode= currNode->next) 
	{
		currToken = currNode->mime;
			
		switch (currToken->tokenType) {

		case PP_DEFINE:
				dodefine(&currNode);
			break;

		case PP_INCLUDE:
			break;
			
		case ID:
			macro = LockupMacro(currToken, &exist, 0);
			if(macro != NULL)
				currNode = expand(currNode, macro);
			break;
		default:
			break;
		}
	}
}




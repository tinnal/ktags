
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

#define ARG_RANGE_BEGAN 0
#define ARG_RANGE_END 1
TokenNode* expand(TokenNode *tokenNode, Macro *macro)
{
	TokenNode *beganTokenNode = tokenNode, *endTokenNode;
	TokenNode *currNode = tokenNode, *prevNode, **ArgsRange;
	TokenNode *bodyNode, *insertPoint, *delNode, *saveNode, *ret;
	MyToken *token, *token1, *token2;
	MyToken *currToken, *newToken;
	int stringSize;
	char* newString;
	TokenNode dSharpList;
	
	// collect actual arguments 
	endTokenNode = currNode;
	token = NextToken(&currNode);
	if (token && token->tokenType=='(') {
		/* macro with args */
		int narg = 0;
		ArgsRange = (TokenNode**)malloc(sizeof(MyToken*) * (macro->argsCount) * 2);
		memset(ArgsRange, 0, sizeof(MyToken*) * (macro->argsCount) * 2);
		token = NextToken(&currNode);
		if (token->tokenType!=')') {
			int err = 0;
			ArgsRange[narg*2+ARG_RANGE_BEGAN] = currNode;
			for (;;) {
				token = NextToken(&currNode);
				if (token->tokenType==')'){
					//kict out whitespace
					ArgsRange[narg*2+ARG_RANGE_END] =
					currNode->prev->mime->tokenType == SPACE?
					currNode->prev:currNode;
					break;
				}
				else if (token->tokenType==',') {
					//kict out whitespace
					ArgsRange[narg*2+ARG_RANGE_END] =
					currNode->prev->mime->tokenType == SPACE?
					currNode->prev:currNode;

					if (++narg > macro->argsCount){
						err++;
						break;
					}
					token = NextToken(&currNode);
					ArgsRange[narg*2+ARG_RANGE_BEGAN] = currNode;
				}			
			}
			if (err) {
				free(ArgsRange);
				error("Syntax error in macro parameters");
				return NULL;
			}

		}
		endTokenNode = currNode; //update endTokenNode if we have argument.
		token = NextToken(&currNode);
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
			if(bodyNode->mime->ppDef != NULL) {
				//token is format arg, translate it into actual argument.
				for(currNode = ArgsRange[bodyNode->mime->ppArgNo*2+ARG_RANGE_BEGAN];
					currNode != ArgsRange[bodyNode->mime->ppArgNo*2+ARG_RANGE_END];
					currNode =  currNode->next){
					insertPoint = InsertToken(insertPoint, currNode->mime);
				}
			} else{
				insertPoint = InsertToken(insertPoint, bodyNode->mime);
			}

			//insertPoint = InsertToken(insertPoint, InstantiateArg(ArgsArray,bodyNode->mime));
			break;
		case PP_DSHARP:
			/*	Pop the prev token we had processed, check if it will merged with the first token we insert.
			etc:
				to ## ken 	-> merge into 'token'
				12 ## +		-> not merge */
			//1. merge two token string.
			token1 = insertPoint->mime;
			//insertPoint = DelTokenNode(insertPoint);
			NextToken(&bodyNode);
			token2 = bodyNode->mime->ppDef == NULL? bodyNode->mime:
				ArgsRange[bodyNode->mime->ppArgNo*2+ARG_RANGE_BEGAN]->mime;
			stringSize = strlen(token1->name)+ strlen(token2->name) +1;
			newString = malloc(stringSize);
			newString[0] = 0;
			strcat(newString, token1->name);
			strcat(newString, token2->name);
			// 2. call lex to check
			dSharpList.next = dSharpList.prev = &dSharpList;
			Lex(NULL, newString, stringSize, &dSharpList); //FIXME: memery leak!
			// 3. check if two token have been merged
			if(dSharpList.next->next->mime->tokenType== EOFILE){
				//two token have been merged!
				//del the org one and insert the merged token.
				insertPoint = DelTokenNode(insertPoint); 
				insertPoint = InsertToken(insertPoint, dSharpList.next->mime);
			} else {
				insertPoint = InsertToken(insertPoint, token2);
			}

			//4. insert the tail token
			if((bodyNode->mime->ppDef != NULL) && 
				(ArgsRange[bodyNode->mime->ppArgNo*2+ARG_RANGE_BEGAN] 
				!= ArgsRange[bodyNode->mime->ppArgNo*2+ARG_RANGE_END])) {
				//token is format arg, translate it into actual argument.
				for(currNode = (ArgsRange[bodyNode->mime->ppArgNo*2+ARG_RANGE_BEGAN]->next);
					currNode != ArgsRange[bodyNode->mime->ppArgNo*2+ARG_RANGE_END];
					currNode =  currNode->next){
					insertPoint = InsertToken(insertPoint, currNode->mime);
				}
			}			
			break;
		case PP_SHARP:
			token1 = NextToken(&bodyNode); //TODO: ERROR
			//token1 = InstantiateArg(ArgsArray,token1);
			newToken = (MyToken*)malloc(sizeof(MyToken));
			memset(newToken, 0 , sizeof(MyToken));
			newToken->genToken1 = token1;
			newToken->tokenType = SCON;

			//Concat all the actual argument to one  token
			//1.cal the size
			if(token1->ppDef != NULL) {
				//token is format arg, translate it into actual argument.
				for(currNode = ArgsRange[token1->ppArgNo*2+ARG_RANGE_BEGAN];
					currNode != ArgsRange[token1->ppArgNo*2+ARG_RANGE_END];
					currNode =  currNode->next){
					stringSize = strlen(currNode->mime->name);
				}

			} else{
				stringSize = strlen(token1->name);
			}
			newToken->name = (char*)malloc(stringSize+3);
			memset(newToken->name, 0, stringSize+3);
			strcat(newToken->name, "\"");
			//2. Concat all the actual argument to one string
			if(token1->ppDef != NULL) {
				//token is format arg, translate it into actual argument.
				for(currNode = ArgsRange[token1->ppArgNo*2+ARG_RANGE_BEGAN];
					currNode != ArgsRange[token1->ppArgNo*2+ARG_RANGE_END];
					currNode =  currNode->next){
					strcat(newToken->name, currNode->mime->name);
				}
			} else{
				strcat(newToken->name, currNode->mime->name);
			}
			strcat(newToken->name, "\"");
			
			insertPoint = InsertToken(insertPoint, newToken);
			break;
		default:
			insertPoint = InsertToken(insertPoint, bodyNode->mime);
			break;
		}			
	}

	//del the original tokens
	delNode=endTokenNode;
	beganTokenNode = beganTokenNode->prev;
	while(delNode != beganTokenNode)
	{
		delNode = DelTokenNode(delNode);
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




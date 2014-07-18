
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
	for(;;)
	{
		int argNo = 0;
		if(token == NULL)
			return;
		if(token->tokenType== NEWLINE)
			break;
		for (argNode=macro->args.next; argNode != &macro->args; 
			argNode =argNode->next ){
			if (strcmp((char*)argNode->mime->name, token->name)==0){
				token->ppDef = argNode->mime;
				token->ppArgNo = argNo;
			}
			argNo++;
		}		
		AddToken(&macro->body, token);
		token = NextToken(currNode);
	}
}


TokenNode* expand(TokenNode *tokenNode, Macro *macro)
{
	TokenNode *beganTokenNode = tokenNode;
	TokenNode *currNode = tokenNode;
	TokenNode *bodyNode, *appendNode, *delNode, *saveNode, *ret;
	MyToken *token, **ArgsArray, *nextToken;
	MyToken *currToken, *newToken;
	char *orgName1, *orgName2;
	
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

	//insert the expanded token
	//ret = appendNode = AddToken(beganTokenNode, token);
	appendNode = currNode->prev;
	ret = beganTokenNode->prev;
	for (bodyNode=macro->body.next; bodyNode != &macro->body; 
		bodyNode =bodyNode->next )
	{
		if(bodyNode->mime->ppDef != NULL){
			appendNode = AddToken(appendNode, ArgsArray[bodyNode->mime->ppArgNo]);
		} else if(bodyNode->mime->tokenType == PP_DSHARP){
			orgName1 = bodyNode->prev->mime->name;
			nextToken = NextToken(&bodyNode);
			if(nextToken == NULL) 
				;//TODO: add error hander
			orgName2 = nextToken->name;
			newToken = (MyToken*)malloc(sizeof(MyToken));
			memset(newToken, 0 , sizeof(MyToken));
			newToken->genToken1 = bodyNode->prev->mime;
			newToken->genToken2 = nextToken;
			newToken->tokenType = ID;
			newToken->name = (char*)malloc(strlen(orgName1)+strlen(orgName2)+1);
			sprintf(newToken->name, "%s%s", orgName1, orgName2);
			appendNode = AddToken(appendNode, newToken);
			//TODO: fix pos info
		} else if(bodyNode->mime->tokenType == PP_SHARP){	
			nextToken = NextToken(&bodyNode);
			if(nextToken == NULL) 
				;//TODO: add error hander
			orgName2 = nextToken->name;

			newToken = (MyToken*)malloc(sizeof(MyToken));
			memset(newToken, 0 , sizeof(MyToken));
			newToken->genToken1 = bodyNode->mime;
			newToken->tokenType = SCON;
			newToken->name = (char*)malloc(strlen(orgName2)+3);
			sprintf(newToken->name, "\"%s\"", nextToken->name, orgName2);
			appendNode = AddToken(appendNode, newToken);
			//TODO: fix pos info
		} else if(bodyNode->next->mime->tokenType != PP_DSHARP){
			appendNode = AddToken(appendNode, bodyNode->mime);
		}				
	}

	//del the original tokens
	for (delNode=beganTokenNode; saveNode != currNode;){
		saveNode = delNode->next;
		DelToken(delNode);
		delNode = saveNode;
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




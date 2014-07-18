
#ifdef HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include "c_token.h"
#include "c_parser.h"

static MyToken* lastToken = NULL;

inline NameScope *NewNameScope(
	NameScope *parents, MyToken *scope, ScopeType type, int level)
{
	NameScope *newScope;
	if((newScope = (NameScope*)malloc(sizeof(NameScope))) == NULL)
		return NULL;
	else
	{
		memset(newScope, 0, sizeof(NameScope));
		newScope->type = type;
		newScope->scopeToken = scope;
		newScope->parents = parents;
		newScope->level;
		newScope->tokenList.next = newScope->tokenList.prev = &newScope->tokenList;
		newScope->child.next = newScope->child.prev = &newScope->child;
	}
	return newScope;
}


inline int AddTokenToScope(NameScope *scope,  MyToken* token)
{
	TokenNode *tokenNode = NULL;
	if((tokenNode = (TokenNode*)malloc(sizeof(TokenNode))) == NULL)
		return -1;
	memset(tokenNode, 0, sizeof(TokenNode));
	token->scope = scope;
	tokenNode->mime = token;
	tokenNode->next = &scope->tokenList;
	//scope->tokenList.prev point to last tokenNode
	tokenNode->prev = scope->tokenList.prev;
	scope->tokenList.prev->next = tokenNode;
	scope->tokenList.prev = tokenNode;
	

	printf("[%4d] Add Token (name:%-15s, type:%-22s) to scope (%-15s)\n", 
		token->startLine,token->name,
		token->attr == ATTR_UNDEF?"UNDEF":
		token->attr == ATTR_FUNC?"FUNC":
		token->attr == ATTR_FUNC_DEF?"FUNC_DEF":
		token->attr == ATTR_FUNC_DECL?"FUNC_DECL":
		token->attr == ATTR_FUNC_REF?"FUNC_REF":
		token->attr == ATTR_VAR_DEF?"VAR_DEF":
		token->attr == ATTR_VAR_REF?"VAR_REF":
		token->attr == ATTR_STRUCT_DEF?"STRUCT_DEF":
		token->attr == ATTR_STRUCT_REF?"STRUCT_REF":
		token->attr == ATTR_ENUM_MEMBER_DEF?"ENUM_DEF":
		token->attr == ATTR_ENUM_MEMBER_REF?"ENUM_REF":
		token->attr == ATTR_STRUCT_MEMBER_DEF?"STRUCT_MEMBER_DEF":
		token->attr == ATTR_STRUCT_MEMBER_REF?"STRUCT_MEMBER_REF":
		token->attr == ATTR_TYPEDEF_DEF?"TYPEDEF_DEF":
		token->attr == ATTR_TYPEDEF_REF?"TYPEDEF_REF":"unknow",
		scope->scopeToken->name);
}


inline int AddNameScope(ScopeNode *scopeList, NameScope *scope)
{
	ScopeNode *scopeNode = NULL;

	printf("[%4d] Add Scope (name:%-15s, type:%-22s) to scope (%-15s)\n", 
		scope->scopeToken->startLine,
		scope->scopeToken->name, 
		scope->type == FUNCTION_SCOPE?"FUNCTION_SCOPE":
		scope->type == STRUCT_SCOPE?"STRUCT_SCOPE":
		scope->type == ANONYMOUS_SCOPE?"ANONYMOUS_SCOPE":"unknow",
		scope->parents->scopeToken->name);

	if((scopeNode = (ScopeNode*)malloc(sizeof(ScopeNode))) == NULL)
		return -1;
	scopeNode->mime = scope;

	scopeNode->next = scopeList;
	//scopeList->prev point to last scopeNode
	scopeNode->prev = scopeList->prev;
	scopeList->prev->next = scopeNode;
	scopeList->prev = scopeNode;
}

MyToken _BASE_TYPE ={"", "!BASE!", 0, 0, 0, 0, 0}, *BASE_TYPE = &_BASE_TYPE;
MyToken _GLOBAL_SCOPE_TOKEN ={"", "!GLOBAL SCOPE!", 0, 0, 0, 0, 0},
	*GLOBAL_SCOPE_TOKEN = &_GLOBAL_SCOPE_TOKEN;
MyToken _typeList = {"", "", 0, 0, 0, 0, 0}, *typeList = &_typeList;

void AddType(MyToken *type)
{
	type->next = typeList;
	typeList = type;
}

int isType(MyToken *type)
{
	MyToken * node= typeList;
	for(node = typeList; node != NULL; node= node->next)
	{
		if(strcmp(node->name, type->name) == 0)
			return 1;
	}
	return 0;
}


static void process_attribute(TokenNode **tokenList);


#define IS_TYPE_QUALIFIER(c)	((c) == C_CONST || (c) == C_RESTRICT || (c) == C_VOLATILE)

#define DECLARATIONS    0
#define RULES           1
#define PROGRAMS        2

#define TYPE_C		0
#define TYPE_LEX	1
#define TYPE_YACC	2


#define BUG(arg)  printf("Bug at %s %s %s\n", __FILE__, __FUNCTION__, __LINE__)


void
C_family(TokenNode *tokenList)
{

	int savelevel;
	NameScope *currScope=NULL, *newScope = NULL;
	int braceLevel  = 0, level = 0;
	int isTypedef = 0;
	MyToken *currToken, *nextToken, *funcToken =NULL;
	TokenNode *currNode = tokenList, *nextNode;
	NameScope globalScope;

	currScope = NewNameScope(NULL, GLOBAL_SCOPE_TOKEN, GLOBAL_SCOPE, level);

	for(currNode = currNode->next;
		currNode->mime->tokenType != EOFILE ;
		currNode= currNode->next) 
	{
		currToken = currNode->mime;
		

		//pick next nonspace token
		nextNode = currNode->next;
		nextToken = nextNode->mime;

retry:

		while(nextToken->tokenType == SPACE ||
			nextToken->tokenType == NEWLINE ||
			nextToken->tokenType == COMMENT)
		{
			nextNode = nextNode->next;
			nextToken = nextNode->mime;
		}

		if(nextToken->tokenType == ATTRIBUTE){
			process_attribute(&nextNode);
			nextNode = nextNode->next;
			nextToken = nextNode->mime;			
			goto retry;
		}
			
		switch (currToken->tokenType) {
		case VOID:
		case INT:
		case LONG:
		case CHAR:
		case FLOAT:
		case DOUBLE:
			currScope->VarType = BASE_TYPE;
			break;
		
		case ENUM:
			currScope->isEnum = 1;
		case STRUCT:
		case UNION:
			//Anonymous struct, refer to source insigh, 
			//we set the scope name and type name to "struct | enum | union".
			if(nextToken->tokenType == '{'){
					currToken->attr = ATTR_STRUCT_DEF;
					AddTokenToScope(currScope, currToken);	
					newScope = NewNameScope(currScope, currToken, STRUCT_SCOPE, level);
					currScope->VarType = currToken;
					currScope->haveStructName = 0;
					AddNameScope(&currScope->child, newScope);
					currScope = newScope;
			}else{
				currScope->haveStructName = 1;
			}	
			break;

		case TYPEDEF:
			currScope->isTypedef = 1;
			break;

		case ID:		/* symbol	*/
			//Check if the token is a typedef type
			//FIXME: When the var's have the same name with a typedef,
			//we infer that it is typedef.
			if(isType(currToken) /*&& currScope->inInitializer == 0*/){
				currToken->attr = ATTR_TYPEDEF_REF;
				AddTokenToScope(currScope, currToken);			
				currScope->VarType = currToken;
				continue;
			}

			//function define or refence or declare
			if(nextToken->tokenType == '(' && level == 0){
				//DEF, REF, DECL check will be delay.
				currToken->attr = ATTR_FUNC;
				funcToken = currToken;
				currToken->type = currScope->VarType;
				AddTokenToScope(currScope, currToken);	
				newScope = NewNameScope(currScope, currToken, FUNCTION_SCOPE, level);
				AddNameScope(&currScope->child, newScope);
				currScope = newScope;
				continue;
			}

			//get struct name if we have.
			if(currScope->haveStructName == 1){
				if(nextToken->tokenType == '{'){
					currToken->attr = ATTR_STRUCT_DEF;
					AddTokenToScope(currScope, currToken);
					currScope->haveStructName = 0;
					currScope->VarType = currToken;
					newScope = NewNameScope(currScope, currToken, STRUCT_SCOPE, level);
					AddNameScope(&currScope->child, newScope);
					currScope = newScope;
				} else {
					currToken->attr = ATTR_STRUCT_REF;
					AddTokenToScope(currScope, currToken);
					currScope->haveStructName = 0;
					currScope->VarType = currToken;
				}
				continue;
			}
			
			// struct member define
			if(	currScope->type == STRUCT_SCOPE){
				if(currScope->parents->isEnum == 1){
					currToken->attr = ATTR_ENUM_MEMBER_DEF;
					//ENUM Member must add in global scope.
					AddTokenToScope(currScope->parents, currToken);
				}else{
					currToken->attr = ATTR_STRUCT_MEMBER_DEF;
					currToken->type = currScope->VarType;
					AddTokenToScope(currScope, currToken);
				}
				continue;
			}

			// var define
			if( currScope->isTypedef == 0 &&
				currScope->inInitializer == 0 &&
				currScope->type != STRUCT_SCOPE &&
				currScope->VarType != NULL){
				currToken->attr = ATTR_VAR_DEF;
				currToken->type = currScope->VarType;
				AddTokenToScope(currScope, currToken);
				continue;
			}

			// type define
			if(currScope->isTypedef == 1)
			{
				currToken->attr = ATTR_TYPEDEF_DEF;
				currToken->type = currScope->VarType;
				AddTokenToScope(currScope, currToken);
				AddType(currToken);
				currScope->isTypedef == 0;
				continue;
			}

			// var or function Reference
			if(nextToken->tokenType == '(')
				currToken->attr = ATTR_FUNC_REF;
			else
				currToken->attr = ATTR_VAR_REF;
			AddTokenToScope(currScope, currToken);
			break;
			
		case '{':  /* } */	
			level++;
			if(newScope != NULL){
				newScope = NULL;
			}else{
				//do/while/for etc. scope
				currScope = NewNameScope(currScope, currToken, ANONYMOUS_SCOPE, level);
				AddNameScope(&currScope->child, currScope);				
			}
			currScope->VarType = NULL;
			currScope->isTypedef = 0;
			currScope->inInitializer = 0;
			break;
			/* { */
		case '}':	
			level--;
			if(currScope->parents != NULL)
				currScope = currScope->parents;
			currScope->isEnum= 0;
			newScope = NULL;
			break;

		case '(': 	
			braceLevel++;
			break;
			
		case ')':	
			if(--braceLevel == 0 && funcToken != NULL)
			{
				if(nextToken->tokenType == '{'){ 
					funcToken->attr = ATTR_FUNC_DEF;
				} else if(nextToken->tokenType == ';'){ 
					funcToken->attr = ATTR_FUNC_DECL;
					if(currScope->parents != NULL)
						currScope = currScope->parents;
				}
				funcToken = NULL;
			}
			currScope->VarType = NULL;
			break;
		case '=':
			currScope->inInitializer = 1;
			break;
		case ',':
			currScope->inInitializer = 0;
			break;
		case ';':
			currScope->VarType = NULL;
			currScope->inInitializer = 0;
			currScope->isTypedef = 0;
			break;
/*
		case '\n':
			if (startmacro && level != savelevel) {
				if (param->flags & PARSER_WARNING)
					warning("different level before and after #define macro. reseted. [+%d %s].", lineno, curfile);
				level = savelevel;
			}
			startmacro = startsharp = 0;
			break;
*/
			
		case ATTRIBUTE:
			process_attribute(&currNode);
			break;
		default:
			break;
		}
	}
}



/*
 * process_attribute: skip attributes in __attribute__((...)).
 */

static void
process_attribute(TokenNode **tokenList)
{
	TokenNode *CurrNode = *tokenList;
	int brace = 0;
	int c;
	
	for(CurrNode = CurrNode->next;
		(c = CurrNode->mime->tokenType) != EOFILE;
		CurrNode= CurrNode->next) 
	{

		while(CurrNode->mime->tokenType == SPACE ||
			CurrNode->mime->tokenType == NEWLINE ||
			CurrNode->mime->tokenType == COMMENT)
		{
			CurrNode = CurrNode->next;
			c = CurrNode->mime->tokenType;
		}
			
		if (c == '(')
			brace++;
		else if (c == ')')
			brace--;
		else if (c == ID) {
//			PUT(PARSER_REF_SYM, token, lineno, sp);
		}
		if (brace == 0){
			*tokenList = CurrNode;
			break;
		}
	}
}




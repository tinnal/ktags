#ifndef C_PARSER_H
#define C_PARSER_H

#include "c_token.h"

typedef enum _ScopeType
{
	GLOBAL_SCOPE,
	STRUCT_SCOPE,
	FUNCTION_SCOPE,
	ANONYMOUS_SCOPE,	
}ScopeType;



struct _NameScope;
typedef struct _ScopeNode
{
	struct _NameScope* mime;
	struct _ScopeNode* next;
	struct _ScopeNode* prev;
}ScopeNode;

typedef struct _NameScope
{
	MyToken *scopeToken;
	MyToken *VarType;
	int inInitializer;
	int haveStructName;
	int isEnum;
	int isTypedef;
	ScopeType type;
	int level;
	struct _NameScope* parents;
	ScopeNode child;
	TokenNode tokenList;	
}NameScope;

void C_family(TokenNode *tokenList);

#endif

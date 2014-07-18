       
enum attribute
{
 ATTR_UNDEF,
 ATTR_FUNC,
 ATTR_FUNC_DEF,
 ATTR_FUNC_DECL,
 ATTR_FUNC_REF,
 ATTR_VAR_DEF,
 ATTR_VAR_REF,
 ATTR_STRUCT_DEF,
 ATTR_STRUCT_REF,
 ATTR_STRUCT_MEMBER_DEF,
 ATTR_STRUCT_MEMBER_REF,
 ATTR_ENUM_MEMBER_DEF,
 ATTR_ENUM_MEMBER_REF,
 ATTR_TYPEDEF_DEF,
 ATTR_TYPEDEF_REF,
};
struct _NameScope;
typedef struct __Token
{
 char* file;
 char* name;
 long startLine, startCol, endLine, endCol;
 int tokenType;
 struct __Token *next;
 int attr;
 struct __Token* type;
 struct _NameScope *scope;
}MyToken;
typedef struct _TokenNode
{
 struct __Token* mime;
 struct _TokenNode* next;
 struct _TokenNode* prev;
}TokenNode;
int OpenTokenFile(char *fileName, TokenNode *tokenList);
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
static MyToken* lastToken = ((void *)0);
inline NameScope *NewNameScope(
 NameScope *parents, MyToken *scope, ScopeType type, int level)
{
 NameScope *newScope;
 if((newScope = (NameScope*)malloc(sizeof(NameScope))) == ((void *)0))
  return ((void *)0);
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
inline int AddTokenToScope(NameScope *scope, MyToken* token)
{
 TokenNode *tokenNode = ((void *)0);
 if((tokenNode = (TokenNode*)malloc(sizeof(TokenNode))) == ((void *)0))
  return -1;
 memset(tokenNode, 0, sizeof(TokenNode));
 token->scope = scope;
 tokenNode->mime = token;
 tokenNode->next = &scope->tokenList;
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
 ScopeNode *scopeNode = ((void *)0);
 printf("[%4d] Add Scope (name:%-15s, type:%-22s) to scope (%-15s)\n",
  scope->scopeToken->startLine,
  scope->scopeToken->name,
  scope->type == FUNCTION_SCOPE?"FUNCTION_SCOPE":
  scope->type == STRUCT_SCOPE?"STRUCT_SCOPE":
  scope->type == ANONYMOUS_SCOPE?"ANONYMOUS_SCOPE":"unknow",
  scope->parents->scopeToken->name);
 if((scopeNode = (ScopeNode*)malloc(sizeof(ScopeNode))) == ((void *)0))
  return -1;
 scopeNode->mime = scope;
 scopeNode->next = scopeList;
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
 for(node = typeList; node != ((void *)0); node= node->next)
 {
  if(strcmp(node->name, type->name) == 0)
   return 1;
 }
 return 0;
}
static void process_attribute(TokenNode *tokenList);
void
C_family(TokenNode *tokenList)
{
 int savelevel;
 NameScope *currScope=((void *)0), *newScope = ((void *)0);
 int braceLevel = 0, level = 0;
 int isTypedef = 0;
 MyToken *currToken, *nextToken, *funcToken =((void *)0);
 TokenNode *currNode = tokenList, *nextNode;
 NameScope globalScope;
 currScope = NewNameScope(((void *)0), GLOBAL_SCOPE_TOKEN, GLOBAL_SCOPE, level);
 for(currNode = currNode->next;
  currNode->mime->tokenType != 3323 ;
  currNode= currNode->next)
 {
  currToken = currNode->mime;
  nextNode = currNode->next;
  nextToken = nextNode->mime;
  while(nextToken->tokenType == 3321 ||
   nextToken->tokenType == 3320 ||
   nextToken->tokenType == 3322)
  {
   nextNode = nextNode->next;
   nextToken = nextNode->mime;
  }
  switch (currToken->tokenType) {
  case 2315:
  case 2289:
  case 2291:
  case 2265:
  case 2280:
  case 2273:
   currScope->VarType = BASE_TYPE;
   break;
  case 2276:
   currScope->isEnum = 1;
  case 2309:
  case 2313:
   if(nextToken->tokenType == '{'){
     currToken->attr = ATTR_STRUCT_DEF;
     AddTokenToScope(currScope, currToken);
     newScope = NewNameScope(currScope, currToken, STRUCT_SCOPE, level);
     currScope->VarType = currToken;
     currScope->haveStructName = 0;
   }else{
    currScope->haveStructName = 1;
   }
   break;
  case 2312:
   currScope->isTypedef = 1;
   break;
  case 4286:
   if(isType(currToken) ){
    currToken->attr = ATTR_TYPEDEF_REF;
    AddTokenToScope(currScope, currToken);
    currScope->VarType = currToken;
    continue;
   }
   if(nextToken->tokenType == '(' && level == 0){
    currToken->attr = ATTR_FUNC;
    funcToken = currToken;
    currToken->type = currScope->VarType;
    AddTokenToScope(currScope, currToken);
    newScope = NewNameScope(currScope, currToken, FUNCTION_SCOPE, level);
    AddNameScope(&currScope->child, newScope);
    currScope = newScope;
    continue;
   }
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
   if( currScope->type == STRUCT_SCOPE){
    if(currScope->parents->isEnum == 1){
     currToken->attr = ATTR_ENUM_MEMBER_DEF;
     AddTokenToScope(currScope->parents, currToken);
    }else{
     currToken->attr = ATTR_STRUCT_MEMBER_DEF;
     currToken->type = currScope->VarType;
     AddTokenToScope(currScope, currToken);
    }
    continue;
   }
   if( currScope->isTypedef == 0 &&
    currScope->inInitializer == 0 &&
    currScope->type != STRUCT_SCOPE &&
    currScope->VarType != ((void *)0)){
    currToken->attr = ATTR_VAR_DEF;
    currToken->type = currScope->VarType;
    AddTokenToScope(currScope, currToken);
    continue;
   }
   if(currScope->isTypedef == 1)
   {
    currToken->attr = ATTR_TYPEDEF_DEF;
    currToken->type = currScope->VarType;
    AddTokenToScope(currScope, currToken);
    AddType(currToken);
    currScope->isTypedef == 0;
    continue;
   }
   if(nextToken->tokenType == '(')
    currToken->attr = ATTR_FUNC_REF;
   else
    currToken->attr = ATTR_VAR_REF;
   AddTokenToScope(currScope, currToken);
   break;
  case '{':
   level++;
   if(newScope != ((void *)0)){
    newScope = ((void *)0);
   }else{
    newScope = NewNameScope(currScope, currToken, ANONYMOUS_SCOPE, level);
    AddNameScope(&currScope->child, newScope);
    currScope = newScope;
   }
   currScope->VarType = ((void *)0);
   currScope->isTypedef = 0;
   currScope->inInitializer = 0;
   break;
  case '}':
   level--;
   if(currScope->parents != ((void *)0))
    currScope = currScope->parents;
   currScope->isEnum= 0;
   newScope = ((void *)0);
   break;
  case '(':
   braceLevel++;
   break;
  case ')':
   if(--braceLevel == 0 && funcToken != ((void *)0))
   {
    if(nextToken->tokenType == '{'){
     funcToken->attr = ATTR_FUNC_DEF;
    } else if(nextToken->tokenType == ';'){
     funcToken->attr = ATTR_FUNC_DECL;
    }
    funcToken = ((void *)0);
   }
   currScope->VarType = ((void *)0);
   break;
  case '=':
   currScope->inInitializer = 1;
   break;
  case ',':
   currScope->inInitializer = 0;
   break;
  case ';':
   currScope->VarType = ((void *)0);
   currScope->inInitializer = 0;
   currScope->isTypedef = 0;
   break;
  default:
   break;
  }
 }
}

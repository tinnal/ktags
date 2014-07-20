#ifndef C_TOKEN_H
#define C_TOKEN_H

typedef enum{
	ADDEQ	=257,
	ANDAND	,
	ANDEQ	,
	ARRAY	,
	DEREF	,
	DIVEQ	,
	DECR	,
	ELLIPSIS,
	EQL		,
	GEQ		,
	INCR	,
	LEQ		,
	LSHIFT	,
	LSHIFTEQ,
	MODEQ	,
	MULEQ	,
	NEQ		,
	OREQ	,
	OROR	,
	POINTER	,
	RSHIFT	,
	RSHIFTEQ,
	SUBEQ	,
	XOREQ	,
	
	//const
	FCON	=5000,
	ICON	,
	SCON	,
	
	//keyword
	ASM		=2000,
	AUTO	,
	BREAK	,
	CASE	,
	CHAR	,
	CONST	,
	CONTINUE,
	DEFAULT	,
	DO		,
	DOUBLE	,
	ELSE	,
	ENUM	,
	EXTERN	,
	FLOAT	,
	FOR		,
	GOTO	,
	IF		,
	INT		,
	LONG	,
	REGISTER,
	RETURN	,
	SHORT	,
	SIGNED	,
	SIZEOF	,
	STATIC	,
	STRUCT	,
	SWITCH	,
	TYPEDEF	,
	UNION	,
	UNSIGNED,
	VOID	,
	VOLATILE,
	WHILE	,
	ATTRIBUTE,	
	INLINE	,
	RESTRICT,
	PP_IF		,
	PP_ELSE		,
	PP_IFDEF	,
	PP_ENDIF	,
	PP_INCLUDE	,
	PP_DEFINE	,
	PP_UNDEF	,
	PP_LINE	,
	PP_ERROR	,
	PP_PRAGMA	,
	PP_DEFINED	,
	PP_LINENO	,
	PP_FILE		,
	PP_DATE		,
	PP_TIME		,
	PP_STDC		,
	PP_EVAL    	,
	PP_SHARP	,
	PP_DSHARP	,
	
	//control
	EOI		=3000,
	NEWLINE ,
	SPACE 	,
	COMMENT ,
	EOFILE 	,
	
	//id
	ID		=4000,
}KWType;



typedef unsigned int uint;

#define	BSIZE	8192

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
	KWType tokenType;
	struct __Token *next;
	
	//pp
	struct __Token *ppDef;
	int ppArgNo;
	struct __Token *genToken1;
	struct __Token *genToken2;

	//Cooked token
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



int Lex(char* fileName, char* buf, int bufLen, TokenNode* tokenList);
int OpenTokenFile(char *fileName, TokenNode *tokenList);
MyToken* NextToken(TokenNode** nextNode);
TokenNode*  AddToken(TokenNode* tokenList,  MyToken* token);
TokenNode* DelTokenNode(TokenNode* delTokenNode);
TokenNode* InsertToken(TokenNode* insertPoint,  MyToken* token);
MyToken* NewToken(char *file, long startLine, long startCol,
	long endLine, long endCol, char* tokenStart, char* tokenEnd, int type);





#endif

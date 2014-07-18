/* $Id: cpp.h,v 1.6 2001/06/04 22:42:04 drh Exp $ */
#include <stdio.h>
#include "c_token.h"

#define	ISDEFINED	01	/* has #defined value */
#define	ISKW		02	/* is PP keyword */
#define	ISUNCHANGE	04	/* can't be #defined in PP */
#define	ISMAC		010	/* builtin macro, e.g. __LINE__ */

typedef struct source {
	char	*filename;	/* name of file of the source */
	int	line;		/* current line number */
	int	lineinc;	/* adjustment for \\n lines */
	unsigned char	*inb;		/* input buffer */
	unsigned char	*inp;		/* input pointer */
	unsigned char	*inl;		/* end of input */
	FILE*	fd;		/* input source */
	int	ifdepth;	/* conditional nesting in include */
	struct	source *next;	/* stack for #include */
} Source;

typedef enum
{
	PP_NONE,
}MacroType;

typedef struct _Macro
{
	MyToken *macroName;
	TokenNode args;
	TokenNode body;
	MacroType type;
	int argsCount;
	struct _Macro* next; 
}Macro;


typedef	struct	includelist {
	char	deleted;
	char	always;
	char	*file;
} Includelist;



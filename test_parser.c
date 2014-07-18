#include <stdio.h>
#include "c_token.h"
#include "c_parser.h"

int main(int argc, char**argv)
{
	FILE* outputFile;
	TokenNode tokenList, *tokenNode;
	MyToken* token;
	tokenList.prev = &tokenList;
	tokenList.next = &tokenList;
	OpenTokenFile(argv[1], &tokenList);
	C_family(&tokenList);

	printf("-------------------------------------------------\n");

	if(argc < 2) return 0;
	outputFile = fopen(argv[2], "w+");
	fprintf(outputFile, "%s", "<html><head> \n\
<link href=\"ktags.css\" rel=\"stylesheet\"type=\"text/css\"/> \n\
<title>Linux kernel - LXR/Linux Cross Reference</title></head> \n\
<body><div id=\"lxrcode\"><pre>\n\
-------------- CSS table ----------------\n\
<span class='c0'> ATTR_UNDEF </span>\n\
<span class='c1'> ATTR_FUNC </span>\n\
<span class='c2'> ATTR_FUNC_DEF </span>\n\
<span class='c3'> ATTR_FUNC_DECL </span>\n\
<span class='c4'> ATTR_FUNC_REF </span>\n\
<span class='c5'> ATTR_VAR_DEF </span>\n\
<span class='c6'> ATTR_VAR_REF </span>\n\
<span class='c7'> ATTR_STRUCT_DEF </span>\n\
<span class='c8'> ATTR_STRUCT_REF </span>\n\
<span class='c9'> ATTR_STRUCT_MEMBER_DEF </span>\n\
<span class='c10'> ATTR_STRUCT_MEMBER_REF </span>\n\
<span class='c11'> ATTR_ENUM_MEMBER_DEF </span>\n\
<span class='c12'> ATTR_ENUM_MEMBER_REF </span>\n\
<span class='c13'> ATTR_TYPEDEF_DEF </span>\n\
<span class='c14'> ATTR_TYPEDEF_REF </span>\n\
---------------- End css table --------------------\n");


	for(tokenNode = tokenList.next; 
		tokenNode != &tokenList; tokenNode = tokenNode->next)
	{
		token = tokenNode->mime;
		if(token->attr != 0)
		{
			fprintf(outputFile, "<span class=\"c%d\">", token->attr);
		}else if(token->tokenType >=2000 && token->tokenType <=3000 ){
			fprintf(outputFile, "%s", "<span class=\"keyword\">");		
		}else if(token->tokenType == FCON || token->tokenType == ICON){
			fprintf(outputFile, "%s", "<span class=\"const\">");
		}else if(token->tokenType == SCON){
			fprintf(outputFile, "%s", "<span class=\"string\">");
		}else if(token->tokenType == COMMENT){
			fprintf(outputFile, "%s", "<span class=\"comment\">");
		}else{
			fprintf(outputFile, "%s", "<span class=\"other\">");
		}
		
		if(token->tokenType == LSHIFTEQ)
			fprintf(outputFile, "%s", "&lt;&lt;=");
		else if(token->tokenType == LSHIFT)
			fprintf(outputFile, "%s", "&lt;&lt;");
		else if(token->tokenType == LEQ)
			fprintf(outputFile, "%s", "&lt;=");
		else if(token->tokenType == '<')
			fprintf(outputFile, "%s", "&lt;");
		else
			fprintf(outputFile, "%s", token->name);
		fprintf(outputFile, "%s", "</span>");
	}


	fprintf(outputFile, "%s", "</pre></div></body></html>");
	fclose(outputFile);
}


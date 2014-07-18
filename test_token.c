#include <stdio.h>
#include "c_token.h"

int main(int argc, char**argv)
{
	FILE* outputFile;
	TokenNode tokenList, *tokenNode;
	MyToken* token;
	tokenList.prev = &tokenList;
	tokenList.next = &tokenList;
	OpenTokenFile(argv[1], &tokenList);

	printf("-------------------------------------------------\n");
	for(tokenNode = tokenList.next; 
		tokenNode != &tokenList; tokenNode = tokenNode->next)
	{
		token = tokenNode->mime;
		printf("Add token %s, %d:%d - %d:%d\n", token->name, 
			token->startLine, token->startCol, token->endLine, token->endCol);
	}

	if(argc < 2) return 0;
	outputFile = fopen(argv[2], "wb+");
	//outputFile = stdout;
	fprintf(outputFile, "<html><head> \n\
<title>Linux kernel - LXR/Linux Cross Reference</title></head> \n\
<body><div id=\"lxrcode\"><pre>\n");


	for(tokenNode = tokenList.next; 
		tokenNode != &tokenList; tokenNode = tokenNode->next)
	{
		token = tokenNode->mime;
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
	}

	fprintf(outputFile, "%s", "</pre></div></body></html>");
	fclose(outputFile);
}


all: test_token test_parser test_pp

test_token: test_token.c c_token.c c_token.h
	gcc -g test_token.c c_token.c -o test_token

test_parser: test_parser.c c_parser.c c_parser.h c_token.c c_token.h
	gcc -g test_parser.c c_parser.c c_token.c -o test_parser

test_pp: test_pp.c c_token.c c_token.h c_pp.h c_pp.c
	gcc -g test_pp.c c_token.c c_pp.c -o test_pp
	
c_token.c: c_token.tem.c
	re2c -o c_token.c c_token.tem.c

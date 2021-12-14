# custom-language-compiler

A script which parses and executes files written in a custom programming language. The compiler scans the text for syntax errors and compiles it into an executable program.

The custom language supports conditional and loop statements, and it evaluates arithmetic expressions. The format of the input file is the variable declaration section first, then the program, then the "command line input" on the last line, which emulates input from the user. 

Here's an example of an input file.



j, i, k ;
{
i = 4;
j = 3;

input k;

WHILE i > 0 {
	j = 3;		
	WHILE j > 0 {
		k = i*j;
		output k;
		j = j-1;
	}
	i = i-1;
}

}
1 2 3 1


The output of the program should be 12 8 4 9 6 3 6 4 2 3 2 1. The input statement on line 6 can be thought of as "cin >> k;", and the last line can be thought of as the command line input. 


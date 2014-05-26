/*======================================/
	Homebrew calculator program
	Author	: Tracy Parsons
	Date	: 5/25/14
/======================================*/

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#define TABWIDTH 8
//#define DEBUG

int line_index;
char ECHO;

struct TokenStruct {
	char *Token;
	char type;
	char priority;
	int index;
};

char TokenType (char in) {	// Note: we only need to check the first character of the string since NextToken won't mix token types
	if ((in >= 'A' && in <= 'Z') || (in >= 'a' && in <= 'z') || in == '_') { return 1; }
	else if ((in >= '0' && in <= '9') || in == '.') { return -1; }
	return 0;
}

char OpPriority (char op) {	// calculator follows standard order of operations 
	if (op == '\n' || op == 0) { return 0; }
	else if (op == ')' || op == ',') { return 1; }
	else if (op == '+' || op == '-') { return 2; }
	else if (op == '*' || op == '/' || op == '%') { return 3; }
	else if (op == '^') { return 4; }
	return -1;
}

double GetNum (char *c) {
	double ret = 0.0; int i = 0;
	while (c[i] >= '0' && c[i] <= '9') {
		ret *= 10.0;
		ret += (double)(c[i++] - '0');
	}
	if (c[i++] == '.') {
		int factor = 10;
		while (c[i] >= '0' && c[i] <= '9') {
			ret += (double)(c[i++] - '0') / (double)factor;
			factor *= 10;
		}
	}
	return ret;
}

struct Variable {
	double Value;
	char flag;
	struct Variable *Next[53];	// 53 entries: A-Z, a-z, _
};

void DisplayVars (struct Variable *Vars, char *Str, int depth) {
	if (!Vars) { return; }
	if (Vars->flag) { printf ("	%s = %f;\n", Str, Vars->Value); }

	Str = realloc (Str, sizeof (char) * (depth + 2));
	Str[depth+1] = 0;

	int i;
	for (i = 0; i < 53; i++) {
		if (Vars->Next[i]) {
			if (i >= 0 && i <= 25) { Str[depth] = i+'A'; }
			else if (i >= 26 && i <= 51) { Str[depth] = i+'a'-26; }
			else { Str[depth] = '_'; }
			DisplayVars (Vars->Next[i], Str, depth + 1);
		}
	}

	Str[depth] = 0;
	return;
}

void AssignValue (char *in, double val, struct Variable *tree) {
	int i = 0;
	struct Variable *cur = tree;
	while (in[i]) {
		if (in[i] >= 'A' && in[i] <= 'Z') {
			if (!cur->Next[in[i]-'A']) {
				cur->Next[in[i]-'A'] = malloc (sizeof (struct Variable));
				cur->Next[in[i]-'A']->flag = 0;
			}
			cur = cur->Next[in[i]-'A'];
		}
		else if (in[i] >='a' && in[i] <= 'z') {
			if (!cur->Next[in[i]-'a'+26]) {
				cur->Next[in[i]-'a'+26] = malloc (sizeof (struct Variable));
				cur->Next[in[i]-'a'+26]->flag = 0;
			}
			cur = cur->Next[in[i]-'a'+26];
		}
		else if (in[i] == '_') {
			if (!cur->Next[52]) {
				cur->Next[52] = malloc (sizeof (struct Variable));
				cur->Next[52] = 0;
			}
			cur = cur->Next[52];
		}
		i++;
	}
	cur->flag = 1;
	cur->Value = val;
	cur = 0;
	return;
}

double GetValue (char *In, struct Variable *Vars) {
	int i = 0;
	struct Variable *Cur = Vars;
	while (In[i] && Cur) {
		if (In[i] >= 'A' && In[i] <= 'Z') { Cur = Cur->Next[In[i]-'A']; }
		else if (In[i] >='a' && In[i] <= 'z') { Cur = Cur->Next[In[i]-'a'+26]; }
		else if (In[i] == '_') { Cur = Cur->Next[52]; }
		i++;
	}
	if (In[i] || !Cur) { return 0.0; }
	
	double ret = Cur->Value;
	Cur = 0;
	return ret;
}

void AddSpace (char c) {
	if (c == '\t') { line_index += (TABWIDTH - (line_index+2) % TABWIDTH); }	// add two spaces for "> " prompt
	else { line_index++; }
	return;
}

char FindToken (char in) {
	char c = in;
	while (c == 0 || c == ' ' || c == '\t') {
		c = getchar ();
		AddSpace (c);
		if (ECHO) { putchar (c); }
	}
	return c;
}

struct TokenStruct NextToken (char *c) {
	*c = FindToken (*c);
	struct TokenStruct ret;// = malloc (sizeof (struct TokenStruct));
	ret.Token = malloc (sizeof (char) * 2);
	ret.Token[0] = *c;
	ret.type = TokenType (*c);
	ret.priority = OpPriority (*c);
	ret.index = line_index;
	
	int length = 1;
	if (!ret.type) {
		if (*c == '+' || *c == '-' || *c == '*' || *c == '/' || *c == '%' || *c == '^' || *c == '=' || *c == ')' || *c == '(' || *c == ',') {	// Grab next character if we have a valid token
			*c = FindToken (0);
		}
		ret.Token[1] = '\0';
#ifdef DEBUG
		if (ret.Token[0] != '\n') { printf ("	Op/Invalid Token %i: '%s';\n", ret.index, ret.Token); }
		else { printf ("	Op/Invalid Token %i: '\\n';\n", ret.index); }
		sleep (1);
#endif
		return ret;
	}
	
	*c = getchar ();
	AddSpace (*c);
	while (TokenType (*c) == ret.type) {
		ret.Token[length++] = *c;
		if (ECHO) { putchar (*c); }
		ret.Token = realloc (ret.Token, sizeof (char) * (length+1));
		*c = getchar ();
		AddSpace (*c);
	}
	if (ECHO) { putchar (*c); }
	//line_index += length;
	ret.Token[length] = '\0';
	*c = FindToken (*c);
#ifdef DEBUG
	printf ("	Var/Num Token %i: %s;\n", ret.index, ret.Token);
	sleep (1);
#endif
	return ret;
}

void HandleError (struct TokenStruct in, char status, char *look) {
	while (*look && *look != '\n') {	// dispose of remaining characters on the line
		*look = getchar ();
		if (ECHO) { putchar (*look); }
	}
	
	if (status) {	// if there's an error, spit out error message and index indicator
		while (--in.index > -2) { printf (" "); }	// *(1-ECHO)
		printf ("^	");
		fflush (0);
		if (in.Token[0] < ' ') {	// replace non-renderable characters with the appropriate ascii label
			switch (in.Token[0]) {
				case 0:
					in.Token = "NUL";
					break;
				case 1:
					in.Token = "SOH";
					break;
				case 2:
					in.Token = "STX";
					break;
				case 3:
					in.Token = "ETX";
					break;
				case 4:
					in.Token = "EOT";
					break;
				case 5:
					in.Token = "ENQ";
					break;
				case 6:
					in.Token = "ACK";
					break;
				case 7:
					in.Token = "BEL";
					break;
				case 8:
					in.Token = "BS";
					break;
	/*			case 9:	// this will never be passed into error handler as blank space is skipped by NextToken
					in.Token = "TAB";
					break;*/
				case 10:
					in.Token = "NEW LINE";
					break;
				case 11:
					in.Token = "VT";
					break;
				case 12:
					in.Token = "FF";
					break;
				case 13:
					in.Token = "CR";
					break;
				case 14:
					in.Token = "SO";
					break;
				case 15:
					in.Token = "SI";
					break;
				case 16:
					in.Token = "DLE";
					break;
				case 17:
					in.Token = "DC1";
					break;
				case 18:
					in.Token = "DC2";
					break;
				case 19:
					in.Token = "DC3";
					break;
				case 20:
					in.Token = "DC4";
					break;
				case 21:
					in.Token = "NAK";
					break;
				case 22:
					in.Token = "SYN";
					break;
				case 23:
					in.Token = "ETB";
					break;
				case 24:
					in.Token = "CAN";
					break;
				case 25:
					in.Token = "EM";
					break;
				case 26:
					in.Token = "SUB";
					break;
				case 27:
					in.Token = "ESC";
					break;
				case 28:
					in.Token = "FS";
					break;
				case 29:
					in.Token = "GS";
					break;
				case 30:
					in.Token = "RS";
					break;
				case 31:
					in.Token = "US";
					break;
			}
		}
		switch (status) {	// output result or error message	
			case 1:
				printf ("ERROR: GIVEN '%s'; EXPECTED '+', '-', '*', '/', OR '^';\n", in.Token);
				break;
				
			case 2:
				printf ("ERROR: GIVEN '%s'; EXPECTED NUMBER, VARIABLE, FUNCTION, OR SUBSTATEMENT;\n", in.Token);
				break;
			
			case 3:
				printf ("ERROR: GIVEN '%s'; BAD THINGS HAVE HAPPENED; REACHED INVALID END OF EVALUATION FUNCTION;\n", in.Token);
				break;
			
			case 4:
				printf ("ERROR: GIVEN '%s'; EXPECTED ASSIGN STATEMENT; REQUIRES VALID VARIABLE STRING AS LEFT OPERAND;\n", in.Token);
				break;
				
			case 5:
				printf ("ERROR: GIVEN '%s'; EXPECTED FUNCTION NAME; CANNOT RESOLVE INTO FUNCTION NAME;\n", in.Token);
				break;
			
			case 6:
				printf ("ERROR: GIVEN '%s'; EXPECTED ',' OR ')' AT END OF ARGUMENT;\n", in.Token);
				break;
			
			case 7:
				printf ("ERROR: GIVEN '%s'; EXPECTED ')' AT END OF SUBSTATEMENT;\n", in.Token);
				break;
				
			case 8:
				printf ("ERROR: GIVEN '%s'; EXPECTED END OF LINE;\n", in.Token);
				break;
			
			case 9:
				printf ("ERROR: INCORRECT NUMBER OF ARGUMENTS FOR FUNCTION;\n");
				break;
			
			case 10:
				printf ("ERROR: GIVEN '%s'; INVALID FUNCTION NAME;\n", in.Token);
		}
	}
	
//	while (*look && *look != '\n') { *look = getchar (); }	// dispose of remaining characters on the line	
	return;
}

double Resolve (struct TokenStruct in, char *status, struct Variable *Vars) {
#ifdef DEBUG
	printf ("Resolve in; in: %s, status: %i;\n", in.Token, *status);
	fflush (0);
#endif
	
	double ret;
	if (in.type == -1) { ret = GetNum (in.Token); }
	else if (in.type == 1) { ret = GetValue (in.Token, Vars); }	// variable; functions dealt with elsewhere
	else {
		*status = 2;
		return;	// in doesn't match any possible values; return nothing
	}
	return ret;
}

double GetFunction (struct TokenStruct in, double *Args, int nargs, char *status) {
#ifdef DEBUG
	printf ("GetFunction in; Name: %s, nargs: %i, status: %i;\n", in.Token, nargs, *status);
#endif
	if (in.type != 1) {
		*status = 5;
		return;
	}

	int i = 0;
	while (in.Token[i]) {	// convert name string to all upper case
		if (in.Token[i] >= 'a' && in.Token[i] <= 'z') { in.Token[i] -= ('a'-'A'); }
		i++;
	}
	
	if (!strcmp (in.Token, "ABS") || !strcmp (in.Token, "ABSOLUTE")) {			// absolute value
		if (nargs == 1) { return abs (Args[0]); }	// if this doesn't work, fabs
		*status = 9;
	}
	else if (!strcmp (in.Token, "ASIN") || !strcmp (in.Token, "ARCSINE")) {			// inverse sine
		if (nargs == 1) { return asin (Args[0]); }
		*status = 9;
	}
	else if (!strcmp (in.Token, "ACOS") || !strcmp (in.Token, "ARCCOSINE")) {			// inverse cosine
		if (nargs == 1) { return acos (Args[0]); }
		*status = 9;
	}
	else if (!strcmp (in.Token, "ATAN") || !strcmp (in.Token, "ARCTANGENT")) {			// inverse tangent
		if (nargs == 1) { return atan (Args[0]); }
		else if (nargs == 2) { return atan2 (Args[0], Args[1]); }
		*status = 9;
	}
	else if (!strcmp (in.Token, "COS") || !strcmp (in.Token, "COSINE")) {		// cosine
		if (nargs == 1) { return cos (Args[0]); }
		*status = 9;
	}
	else if (!strcmp (in.Token, "DIST") || !strcmp (in.Token, "DISTANCE") || !strcmp (in.Token, "HYPOTENUSE")) {	// pythagorian distance / hypotenuse
		if (nargs == 2) { return sqrt (pow (Args[0], 2) + pow (Args[1], 2)); }
		*status = 9;
	}
	else if (!strcmp (in.Token, "E")) {
		if (!nargs) { return M_E; }
		*status = 9;
	}
	else if (!strcmp (in.Token, "EXP")) {
		if (nargs == 1) { return exp (Args[0]); }		
		*status = 9;
	}
	else if (!strcmp (in.Token, "LN") || !strcmp (in.Token, "NATURAL_LOG")) {	// log base Arg[1] of Arg[0]
		if (nargs == 1) { return log (Args[0]); }
		*status = 9;
	}
	else if (!strcmp (in.Token, "LOG") || !strcmp (in.Token, "LOGARITHM")) {	// log base Arg[1] of Arg[0]
		if (nargs == 1) { return log (Args[0]); }
		else if (nargs == 2) { return log (Args[0]) / log (Args[1]); }
		*status = 9;
	}
	else if (!strcmp (in.Token, "MOD") || !strcmp (in.Token, "MODULUS") || !strcmp (in.Token, "REMAINDER")) {	// remainder of Arg[0] / Arg[1]
		if (nargs == 2) { return fmod (Args[0], Args[1]); }
		*status = 9;
	}
	else if (!strcmp (in.Token, "PI")) {	// return pi
		if (!nargs) { return M_PI; }
		*status = 9;
	}
	else if (!strcmp (in.Token, "POW") || !strcmp (in.Token, "POWER")) {		// power (technically redundant)
		if (nargs != 2) { return pow (Args[0], Args[1]); }
		*status = 9;
	}
	else if (!strcmp (in.Token, "PROD") || !strcmp (in.Token, "PRODUCT")) {		// product of arguments (technically redundant)
		if (nargs) {
			double ret = 1.0;
			for (i = 0; i < nargs; i++) { ret *= Args[i]; }
			return ret;
		}
		*status = 9;
	}
	else if (!strcmp (in.Token, "SIN") || !strcmp (in.Token, "SINE")) {			// sine
		if (nargs == 1) { return cos (Args[0]); }
		*status = 9;
	}
	else if (!strcmp (in.Token, "SQRT") || !strcmp (in.Token, "SQUARE_ROOT")) {	// square root (technically redundant)
		if (nargs == 1) { return sqrt (Args[0]); }
		*status = 9;
	}
	else if (!strcmp (in.Token, "SUM") || !strcmp (in.Token, "SUMMATION")) {	// sum of arguments (technically redundant)
		if (nargs) {
			double ret = 0.0;
			for (i = 0; i < nargs; i++) { ret += Args[i]; }
			return ret;
		}
		*status = 9;
	}
	else if (!strcmp (in.Token, "TAN") || !strcmp (in.Token, "TANGENT")) {	// tangent (technically redundant)
		if (nargs == 1) { return sin (Args[0]) / cos (Args[0]); }
		*status = 9;
	}
	
	if (!*status) { *status = 10; }
	return;
}

double EvalSubStatement (char *status, char *look, struct Variable *Vars);
double EvalFunction (struct TokenStruct val, char *status, char *look, struct Variable *Vars);

double Evaluate (double acc, struct TokenStruct op, char *status, char *look, struct Variable *Vars) {
#ifdef DEBUG
	printf ("Evaluate in; acc: %f, op: %s, status: %i, look: %c;\n", acc, op.Token, *status, *look);
#endif
/*	if (*status > 0) {	// if there are errors from other routines, handle error
		return;
	}*/
	if (!op.priority) {	// no operator present; return passed in value, if valid
		if (!*status) { return acc; }
		else if (*status < 0) { *status += 8; }
		HandleError (op, *status, look);
		return;
	}
	else if (op.priority == -1) {			// invalid operator
		*status = 1;
		HandleError (op, *status, look);
		return;
	}
	else if (op.priority == 1) {
		if (op.Token[0] == ')' && *status < 0) {
			*status = 0;
			return acc;
		}
		else if (*status == -2 && op.Token[0] == ',') { return acc; }	// end of argument
		else { *status += 8; }						// ',' after substatement (-1 -> 7) or other (0 -> 8)
		HandleError (op, *status, look);
		return;
	}
	
	struct TokenStruct val = NextToken (look);
	double cur;
	char neg = 0;
	if (val.Token[0] == '-') {	// if value is negative (has - preceding it), evaluate as negative and get next token
		neg = 1;
		val = NextToken (look);
	}
	
	if (val.Token[0] == '(') { cur = EvalSubStatement (status, look, Vars);	}	// sub statement
	if (*status > 0) { return; }		// invalid sub statement
	
	struct TokenStruct nop = NextToken (look);						// next operator
	
	if (nop.Token[0] == '(') {	// Function; TokenType (*Val) > 0 && 
		cur = EvalFunction (val, status, look, Vars);
		nop = NextToken (look);					// get operator after function
	}
	else if (val.Token[0] != '(') { cur = Resolve (val, status, Vars); }		// not a function or substatement, so resolve normally

	if (*status > 0) {		// invalid function or value
		HandleError (val, *status, look);
		return;
	}
	
	if (neg) { cur *= -1.0; }	// account for negative numbers
	
	switch (op.Token[0]) {
		case '+':
			if (nop.priority <= op.priority) { return Evaluate (acc + cur, nop, status, look, Vars); }	// operator has equal or greater precedence than next operator
			else { return acc + (Evaluate (cur, nop, status, look, Vars)); }						// operator has less precedence than next operator
			
		case '-':
			if (nop.priority <= op.priority) { return Evaluate (acc - cur, nop, status, look, Vars); }
			else { return acc - (Evaluate (cur, nop, status, look, Vars)); }
		
		case '*':
			if (nop.priority <= op.priority) { return Evaluate (acc * cur, nop, status, look, Vars); }
			else { return acc * (Evaluate (cur, nop, status, look, Vars)); }
			
		case '/':
			if (nop.priority <= op.priority) { return Evaluate (acc / cur, nop, status, look, Vars); }
			else { return acc / (Evaluate (cur, nop, status, look, Vars)); }
		
		case '%':
			if (nop.priority <= op.priority) { return Evaluate (fmod (acc, cur), nop, status, look, Vars); }
			else { return fmod (acc, (Evaluate (cur, nop, status, look, Vars))); }

		case '^':
			if (nop.priority <= op.priority) { return Evaluate (pow (acc, cur), nop, status, look, Vars); }
			else { return pow (acc, (Evaluate (cur, nop, status, look, Vars))); }
	}
	
	*status = 1;
	HandleError (op, *status, look);
	return;	// invalid operator
}

double EvalSubStatement (char *status, char *look, struct Variable *Vars) {
#ifdef DEBUG
	printf ("EvalSubStatement in; status: %i, look: %c;\n", *status, *look);
#endif
	char oldstatus = *status;
	*status = -1;					// pass on substatement status
	
	struct TokenStruct tmp;
//	tmp.Token = malloc (sizeof (char) * 2);
	tmp.Token = "+";
	tmp.type = TokenType ('+');
	tmp.priority = OpPriority ('+');
	
	double ret = Evaluate (0, tmp, status, look, Vars);
	if (!*status) { *status = oldstatus; }	// if no error occured during substatement evaluation, reset status

	return ret;
}

double EvalFunction (struct TokenStruct val, char *status, char *look, struct Variable *Vars) {
#ifdef DEBUG
	printf ("EvalFunction in; Val: %s, status: %i, look: %c;\n", val.Token, *status, *look);
#endif

	// parse args
	double *Args = 0; int nargs = 0;
	char oldstatus = *status;
	struct TokenStruct tmp;
	//tmp.Token = malloc (sizeof (char) * 2);
	tmp.Token = "+";
	tmp.type = TokenType ('+');
	tmp.priority = OpPriority ('+');
//	tmp.index = 0;
	
	*status = -2;	// set status to indicate argument evaluation 
	if (*look != ')') {	// make sure there are more than zero arguments
		while (*status == -2) {
			Args = realloc (Args, sizeof (double) * ++nargs);			// make room for the next argument
			Args[nargs-1] = Evaluate (0.0, tmp, status, look, Vars);	// and add it to the argument vector
		}
		
		if (*status) {		// argument(s) invalid; return. Error in Evaluate, let it handle the error
			free (Args);
			return;
		}
	}
	else { NextToken (look); }
	*status = oldstatus;
	
	// resolve function
	double ret = GetFunction (val, Args, nargs, status);

	if (Args) { free (Args); }
//	if (*status > 0) { HandleError (val, *status, look); }	// Let Evaluate or Parse handle errors
	return ret;
}

char Parse (struct Variable *Vars) {	// grabs first two tokens to see if they are an assign statment
	// Symbol-specified functions:
	//		= (Assign), + (Add), - (Subtract), * (Multiply), / (Divide), % (Modulus), ^ (Power)
	// Future symbols: ! (Factorial / Gamma function), 
	// Unused symbols: ~, `, @, #, $, &, [, ], {, }, |, \, ;, :, ', ", ?, ,, <, >,
	line_index = 0;
	char look = 0, status = 0;
	
	printf ("> ");
	fflush (0);
	
	struct TokenStruct /*Oper*/and = NextToken (&look);
	
	// check for calculator commands
	if (!strcmp (and.Token, "DISPLAY")) {	// display variable tree
		char *disp = 0;
		DisplayVars (Vars, disp, 0);
		HandleError (and, 0, &look);
		return 0;
	}
	else if (!strcmp (and.Token, "ECHO")) {	// toggle input echo mode
		ECHO = 1 - ECHO;
		if (ECHO) { puts ("Input echo on"); } else { puts ("Input echo off"); }
		HandleError (and, 0, &look);
		return 0;
	}
	else if (!strcmp (and.Token, "HELP")) {	// display help screen
		puts ("Basic Calculator V 0.9");
		puts ("--------------------------------------------------------------------------------");
		puts ("This program is meant to be used as a general purpose calculator, akin to bc. Users can input arithmitic expressions of arbitrary length, assign and retrieve variables, and calculate common math functions. If the expression input isn't valid, the program displays an error message, indicates where in the line the problem occured, clears the input and resumes operation. In addition, users can work in interactive mode or redirect input scripts to execute a list of operations.");
		puts ("--------------------------------------------------------------------------------");
		puts ("COMMANDS: (case sensitive)");
		puts ("	DISPLAY	:	Display defined variables and their associated values.");
		puts ("	ECHO	:	Echo user input on next line. Useful when using an input script."); 
		puts ("	HELP	:	Display this help screen.");
		puts ("	QUIT	:	Quit this calculator program.");
		puts ("--------------------------------------------------------------------------------");
		puts ("MATH FUNCTIONS: (not case sensitive)\n");
		puts ("ABS / ABSOLUTE	:\n	return the absolute value of the argument. Accepts one argument.");
		puts ("ASIN / ARCSINE	:\n	return the arc sine (inverse sine function) of the argument. Accepts one argument.");
		puts ("ACOS / ARCCOSINE	:\n	return the arc cosine (inverse cosine function) of the argument. Accepts one argument.");
		puts ("ATAN / ARCTANGENT	:\n	return the arc tangent (inverse tangent function) of the argument. Accepts one argument.");
		puts ("COS / COSINE	:\n	return the cosine of the argument. Accepts one argument.");
		puts ("DIST / DISTANCE / HYPOTENUSE	:\n	return the pythangorian distance (hypotenuse) between two values. Accepts two arguments.");
		puts ("E	:\n	return the value of e. Accepts no arguments.");
		puts ("EXP	:\n	return the exponent of the argument (e ^ arg). Accepts one argument.");
		puts ("LN / NATURAL_LOG	:\n	return the natural log of the argument (arg = e ^ x). Accepts one argument.");
		puts ("LOG / LOGARITHM	:\n	return the natural log of an argument OR the log of one argument in terms of another (log A / log b). Accepts one or two arguments.");
		puts ("MOD / MODULUS	:\n	return the remainder of one argument divided by another. Accepts two arguments.");
		puts ("PI	:\n	return the value of pi. Accepts no arguments.");
		puts ("POW / POWER	:\n	return the value of one argument raised to the power of the other. Accepts two arguments.");
		puts ("PROD / PRODUCT	:\n	return the product of the arguments. Accepts one or more arguments.");
		puts ("SIN / SINE	:\n	return the sine of the argument. Accepts one argument.");
		puts ("SQRT / SQUARE_ROOT	:\n	return the square root of the argument. Accepts one argument.");
		puts ("SUM / SUMMATION	:\n	return the sum of the arguments. Accepts one or more arguments.");
		puts ("TAN / TANGENT	:\n	return the tangent of the argument. Accepts one argument.");
		
		HandleError (and, 0, &look);
		return 0;
	}
	else if (!strcmp (and.Token, "QUIT")) {	// quit calculator
		HandleError (and, 0, &look);
		return 1;
	}
	
	struct TokenStruct /*Oper*/ator;
	struct TokenStruct tmp;
	tmp.Token = malloc (sizeof (char) * 2);
	tmp.Token = "+";
	tmp.type = TokenType ('+');
	tmp.priority = OpPriority ('+');
	
	if (!and.priority) { return 0; }		// no operand; blank line

	double result = 0.0;
	double sign = 1.0;
	if (and.Token[0] == '-') {				// negative value
		sign = -1.0;
		and = NextToken (&look);
	}
	
	if (and.Token[0] == '(') { result = sign * EvalSubStatement (&status, &look, Vars); }		// substatement	
	if (status) { return 0; }

	ator = NextToken (&look);				// must be determined *after* substatement, and *before* function, assignment, or variable
	
	// function, assignment, or variable; need operator to decide
	if (ator.Token[0] == '(') {				// function
		result = sign * EvalFunction (and, &status, &look, Vars);
		if (status) {
			if (status == 5 || status == 9) { HandleError (and, status, &look); }
			return 0;
		}
		
		ator = NextToken (&look);
		result = Evaluate (result, ator, &status, &look, Vars);
	}
	else if (and.type == 1 && ator.Token[0] == '=') { result = Evaluate (0.0, tmp, &status, &look, Vars); }	// assignment; don't need to check status here since it will be checked as soon as this statement is finished
	else {	// variable, number, substatement, or invalid
		if (and.Token[0] != '(') {
			result = sign * Resolve (and, &status, Vars);
			if (status) {
				HandleError (and, status, &look);
				return 0;
			}
		}
		result = Evaluate (result, ator, &status, &look, Vars);
	}

	if (status) { return 0; }
	else if (ator.Token[0] == '=') {
		AssignValue (and.Token, result, Vars);
		printf ("%s ", and.Token);
	}
	printf ("= %f;\n", result);
	return 0;
}

int main () {
	ECHO = 0;
	puts ("Basic Homebrew Calculator; Type HELP for help");
	
	struct Variable *Vars = malloc (sizeof (struct Variable));
	while (!Parse (Vars));	// if quit has not been input (parse returns 0), quit. Otherwise, repeat
	return 0;
}

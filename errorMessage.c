/*
 *	errorMeesage - a function to print the problem detected during the program.
 *	the function gets as parameters the file, the number of the line in file
 *	and the index in the line where the problem was found. also she gets the 
 *	expectation and the exact errorFlag. 
 * 	the function print an error message and quit.
 * 	if the flag isn't an errorFlag the function prints that a false call was made.
 *
 *
 */
#include "asmutils.c"
#include "asmutils.h"


int errorMeesage (char* file, int lineCounter, int Index, Expectation *Expecting, Flag flag) /*Note: before the function, turn file from FILE to char* by relevant function.*/
{

	if (flag == ErrorLineLengthFlag)
		{
				printf("ERROR: in File %s, Line %d: line legal length is %d, and Line length is %d", file, lineCounter, SOURCE_LINE_LENGTH, Index);
				return 0;
		}

	if (*Expecting == ExpectDigitOrSign && flag == IllegalSpacingFlag)
		{
		printf("ERROR: in File %s, Line %d, index %d: Expect for digit or sign, and found an illegally positioned space or tab.\n", file, lineCounter, Index);
			return 0;
		}

	if (flag == StrayCommentFlag)
		{
		printf("ERROR: in File %s, Line %d, index %d: found a comment in mid of line, but comment must have a dedicated line .\n", file, lineCounter, Index);
			return 0;
		}

	if (flag == StraySignFlag)
		{
		printf("ERROR: in File %s, Line %d, index %d: Found an illegally positioned plus or minus.\n",file, lineCounter, Index);
			return 0;
		}

	if (flag == StrayDigitFlag)
		{
		printf("ERROR: in File %s, Line %d, index %d: Found an illegally positioned digit \n", file, lineCounter, Index);
			return 0;
		}

	if (flag == StrayCommaFlag)
		{
		printf("ERROR: in File %s, Line %d, index %d: Found an illegally positioned comma.\n", file, lineCounter, Index);
			return 0;
		}		
	if (flag == UnexpectedFlag) /*Note: inQuote, unexpected token,  */
		{
		printf("ERROR: in File %s, Line %d, index %d: found an unexpected character.\n",file, lineCounter, Index);
			return 0;
		}		

	if (flag == HardwareErrorFlag)
		{
		printf("ERROR: in File %s, Line %d, index %d: the function had call for memory, but no room left in the Hardware.\n", file, lineCounter, Index);
			return 0;
		}		

	if (*Expecting == Expect8BitParams && flag == SizeOverflowFlag)
		{
		printf("ERROR: in File %s, Line %d, index %d: Expect for number in 8bit size, and found a bigger size. \n",file, lineCounter, Index);
			return 0;
		}		

	if (*Expecting == Expect16BitParams && flag == SizeOverflowFlag)
		{
		printf("ERROR: in File %s, Line %d, index %d: Expect for number in 16bit size, and found a bigger size.\n", file, lineCounter, Index);
			return 0;
		}		

	if (*Expecting == Expect32BitParams && flag == SizeOverflowFlag) /* Note: for if (value < MIN_SIGNED_HALF || value > MAX_SIGNED_HALF), line 826*/
		{
		printf("ERROR: in File %s, Line %d, index %d: Expect for number in 32bit size, and found a bigger size.\n", file, lineCounter, Index);
			return 0;
		}		

	if (flag == IncompleteStringFlag)
		{
			printf("ERROR: in File %s, Line %d, index %d: Expect for completed string, and found an incomplete string.\n", file, lineCounter, Index);
			return 0;
		}		

	if (flag == InvalidRegisterFlag)
		{
		printf("ERROR: in File %s, Line %d, index %d: found an invalid register.\n", file, lineCounter, Index);
			return 0;
		}		

	if (flag == IllegalSpacingFlag)
		{
		printf("ERROR: in File %s, Line %d, index %d: Found illegally positioned space or tab. \n", file, lineCounter, Index);
			return 0;
		}		

	if (flag == StrayDollarSignFlag)
		{
		printf("ERROR: in File %s, Line %d, index %d: Found illegally positioned dollar sign.\n", file, lineCounter, Index);
			return 0;
		}		

	if (flag == IllegalSymbolFlag) /*Note: start with digit, longer then 31*/
		{
		printf("ERROR: in File %s, Line %d, index %d: Label symbols cannot start with a digit.\n", file, lineCounter, Index);
			return 0;
		}		
	/* else - no error has found in the program. false call to the function */
	printf("False call to the function: no error was found\n");
	return 0;
}

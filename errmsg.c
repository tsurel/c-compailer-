#include "asmutils.h"

/*
 *	errorMeesage - a class to print the problem detected during the program.
 *	this function gets as parameters the file name, the number of the line in file
 *	and the index in the line where the problem was found. also she gets the 
 *	expectation and the exact errorFlag. 
 * 	the function print the correct "error message" and quit.
  *
 */

void errmsgForExtractLine (FILE file, char* fileName, char* line, int lineCounter, int Index, Expectation *Expecting, Flag flag) /*Note: before the function, turn file from FILE to char* by relevant function.*/
{
	int i;
	char sign = '^';
	char space = ' ';
	
	if (flag == WarningLineLengthFlag)
	{
		printf("File %s, Line %d: Warning -line legal length is %d, and Line length is %d", fileName, lineCounter, SOURCE_LINE_LENGTH, Index);
		return;
	}


	if (flag == ErrorLineLengthFlag)
	{
		printf("File %s, Line %d: Error -line legal length is %d, and Line length is %d", fileName, lineCounter, SOURCE_LINE_LENGTH, Index);
		printf("%s\n", line);
		for (i = 0; i < Index; ++i)
		{
			printf("%c", space);
		}
		printf("%c\n", sign);
		return;
	}
}

void errmsgForRangeData (FILE file, char* fileName, char* line, int lineCounter, int Index, Expectation *Expecting, Flag flag)
{
	int i;
	char sign = '^';
	char space = ' ';
	
	if (flag == IllegalSpacingFlag && *Expecting == ExpectDigitOrSign)
	{
		printf("in %s.%d.%d: Error - Expect for digit or sign, and found an illegally positioned space or tab. delete this sign.\n", fileName, lineCounter, Index);
			printf("%s\n", line);
		for (i = 0; i < Index; ++i)
		{
			printf("%c", space);
		}
		printf("%c\n", sign);
		return;
	}

	if (flag == StrayCommentFlag)
	{
		printf("in %s.%d.%d: Error - Found a comment in middle of line, but comment must have a dedicated line. delete this sign.\n", fileName, lineCounter, Index);
			printf("%s\n", line);
		for (i = 0; i < Index; ++i)
		{
			printf("%c", space);
		}
		printf("%c\n", sign);
		return;
	}

	if (flag == StraySignFlag)
	{
		printf("in %s.%d.%d: Error - Found an illegally positioned plus or minus. delete this sign.\n",fileName, lineCounter, Index);
			printf("%s\n", line);
		for (i = 0; i < Index; ++i)
		{
			printf("%c", space);
		}
		printf("%c\n", sign);
		return;
	}

	if (flag == StrayDigitFlag)
	{
		printf("in %s.%d.%d: Error - Found an illegally positioned digit. delete this sign.\n", fileName, lineCounter, Index);
			printf("%s\n", line);
		for (i = 0; i < Index; ++i)
		{
			printf("%c", space);
		}
		printf("%c\n", sign);
		return;
	}

	if (flag == StrayCommaFlag)
	{
		printf("in %s.%d.%d: Error - Found an illegally positioned comma. delete this sign.\n", fileName, lineCounter, Index);
			printf("%s\n", line);
		for (i = 0; i < Index; ++i)
		{
			printf("%c", space);
		}
		printf("%c\n", sign);
		return;
	}	

	if (flag == UnexpectedFlag) /*Note: inQuote, unexpected token - SPLIT!  */
	{
		printf("in %s.%d.%d: Error - found an unexpected character.\n",fileName, lineCounter, Index);
		printf("%s\n", line);
		for (i = 0; i < Index; ++i)
		{
			printf("%c", space);
		}
		printf("%c\n", sign);
		return;
	}		

}

void errmsgForExtractData (FILE file, char* fileName, char* line, int lineCounter, int Index, Expectation *Expecting, Flag flag)
{
	int i;
	char sign = '^';
	char space = ' ';

	if (flag == HardwareErrorFlag)
	{
		printf("in %s.%d.%d: Error - the function had call for memory, but no room left in the Hardware.\n", fileName, lineCounter, Index);
			printf("%s\n", line);
		for (i = 0; i < Index; ++i)
		{
			printf("%c", space);
		}
		printf("%c\n", sign);
		return;
	}		

	if (*Expecting == Expect8BitParams && flag == SizeOverflowFlag)
	{
		printf("in %s.%d.%d: Error - Expect for number in 8bit size, and found a bigger size. \n",fileName, lineCounter, Index);
			printf("%s\n", line);
		for (i = 0; i < Index; ++i)
		{
			printf("%c", space);
		}
		printf("%c\n", sign);
		return;
	}		

	if (*Expecting == Expect16BitParams && flag == SizeOverflowFlag)
	{
		printf("in %s.%d.%d: Error - Expect for number in 16bit size, and found a bigger size.\n", fileName, lineCounter, Index);
			printf("%s\n", line);
		for (i = 0; i < Index; ++i)
		{
			printf("%c", space);
		}
		printf("%c\n", sign);
		return;
	}		

	if (*Expecting == Expect32BitParams && flag == SizeOverflowFlag) /* Note: for if (value < MIN_SIGNED_HALF || value > MAX_SIGNED_HALF), line 826*/
	{
		printf("in %s.%d.%d: Error - Expect for number in 32bit size, and found a bigger size.\n", fileName, lineCounter, Index);
		printf("%s\n", line);
		for (i = 0; i < Index; ++i)
		{
			printf("%c", space);
		}
		printf("%c\n", sign);
		return;
	}		
}

void errmsgForRangeAsciz (FILE file, char* fileName, char* line, int lineCounter, int Index, Expectation *Expecting, Flag flag)
{
	int i;
	char sign = '^';
	char space = ' ';

	if (flag == IncompleteStringFlag)
	{
		printf("in %s.%d.%d: Error - Expect for a quotes in the end of the string.\n", fileName, lineCounter, Index);
		printf("%s\n", line);
		for (i = 0; i < Index; ++i)
		{
			printf("%c", space);
		}
		printf("%c\n", sign);
		return;
	}		

	if(flag == UnexpectedFlag)
	{
		printf("in %s.%d.%d: Error -Unexpected characters have appeared outside the string. delete them. \n", fileName, lineCounter, Index);
		printf("%s\n", line);
		for (i = 0; i < Index; ++i)
		{
			printf("%c", space);
		}
		printf("%c\n", sign);
		return;
	}	
}

void errmsgForRangeParam (FILE file, char* fileName, char* line, int lineCounter, int Index, Expectation *Expecting, Flag flag)
{
	int i;
	char sign = '^';
	char space = ' ';

	if (flag == IllegalSpacingFlag)
	{
		printf("in %s.%d.%d: Error - Found illegally positioned space or tab. delete this space. \n", fileName, lineCounter, Index);
		printf("%s\n", line);
		for (i = 0; i < Index; ++i)
		{
			printf("%c", space);
		}
		printf("%c\n", sign);
		return;
	}		

	if (flag == StrayDollarSignFlag) /*Found illegally positioned dollar sign*/ 
	{
		printf("in %s.%d.%d: Error - Found illegally positioned dollar sign.\n", fileName, lineCounter, Index);
		printf("%s\n", line);
		for (i = 0; i < Index; ++i)
		{
			printf("%c", space);
		}
		printf("%c\n", sign);
		return;	}	

	if (flag == StrayDigitFlag)
	{
		printf("in %s.%d.%d: Error - Found an illegally positioned digit. delete this sign.\n", fileName, lineCounter, Index);
		printf("%s\n", line);
		for (i = 0; i < Index; ++i)
		{
			printf("%c", space);
		}
		printf("%c\n", sign);
		return;
	}

	if (flag == StrayCommaFlag)
	{
		printf("in %s.%d.%d: Error - Found an illegally positioned comma. delete this sign.\n", fileName, lineCounter, Index);
		printf("%s\n", line);
		for (i = 0; i < Index; ++i)
		{
			printf("%c", space);
		}
		printf("%c\n", sign);
		return;
	}	
	if (flag == UnexpectedFlag)
	{
		printf("in %s.%d.%d: Error -Unexpected characters have appeared. delete them. \n", fileName, lineCounter, Index);
		printf("%s\n", line);
		for (i = 0; i < Index; ++i)
		{
			printf("%c", space);
		}
		printf("%c\n", sign);
		return;
	}	
}

void errmsgForExtractParam (FILE file, char* fileName, char* line, int lineCounter, int Index, Expectation *Expecting, Flag flag)
{
	int i;
	char sign = '^'
	char space = ' ';

	if (flag == InvalidRegisterFlag)
	{
		printf("in %s.%d.%d: Error - found an invalid register. The operand is not valid. \n", fileName, lineCounter, Index);
		printf("%s\n", line);
		for (i = 0; i < Index; ++i)
		{
			printf("%c", space);
		}
		printf("%c\n", sign);
		return;
	}		

	if (flag == IllegalSymbolFlag) /*Note: start with digit, longer then 31*/
	{
		printf("in %s.%d.%d: Error - Label symbols cannot start with a digit.\n", fileName, lineCounter, Index);
		printf("%s\n", line);
		for (i = 0; i < Index; ++i)
		{
			printf("%c", space);
		}
		printf("%c\n", sign);
		return;
	}	

	if (flag == HardwareErrorFlag)
	{
		printf("in %s.%d.%d: Error - the function had call for memory, but no room left in the Hardware.\n", fileName, lineCounter, Index);
		printf("%s\n", line);
		for (i = 0; i < Index; ++i)
		{
			printf("%c", space);
		}
		printf("%c\n", sign);
		return;
	}

	if (flag == SizeOverflowFlag)
	{
		printf("in %s.%d.%d: Warning - The immediate value has overflowed 2 bytes.\n", fileName, lineCounter, Index);
		printf("%c\n", line);
		return;
	}		
	/*Note: expectations problems - warning message */	
}
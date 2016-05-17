#ifndef ERRORS_H_
#define ERRORS_H_

/*
	Helper class that includes enum of errors and a struct containing error code and message, used for PCB.c

	@version 2
	4/8/16
	@author Joshua Cho
  @author Mat Sharff
*/

enum err {
	NO_ERRORS, // 0
	NULL_POINTER, // 1
	INVALID_INPUT // 2
};

void print_error(int);

#endif

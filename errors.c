#include <stdio.h>
#include "errors.h"

const char * error_names[3] = {
  "No error.",
  "Null pointer passed.",
  "Invalid Input passed."
};

void print_error(int error_num) {	//prints error message
  printf("%s", error_names[error_num]);
}

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>

#define is_hex(val) (!(!(val>='A' && val<= 'F') && !(val>='a' && val<='f') && !(val>= '0' && val <= '9')))

char* ufgets(FILE* file, size_t* length);
bool is_valid_digits(char*, char**);
#pragma once
#include "stdio.h"

extern int print(const char *, ...);
extern int println(const char *, ...);

extern char * format(const char *, ...);

extern int writef(FILE *, const char *, ...);

//#define _DEBUG

#include "../include/fmt.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

enum {
	INT, LONG, LLONG,
	UINT, ULONG, ULLONG,
	CHAR, BOOL,
	HEX,
	DOUBLE, LDOUBLE,
	PTR, STR
};

struct FMT {
	long long int i;
	double d;
	void * ptr;	
	int hex;

	char repeat;
	char var_length;
	char show_sign;
	char stop;
	unsigned int pointer; // size of array
	
	unsigned int count;
	unsigned int type;

	char * delim;
	unsigned int d_length;
	unsigned int str_length;

	FILE * fp;
};

struct FMT * init_fmt();

/*		FORMAT FUNCTIONS	*/

char * fmt_llong(long long int, struct FMT *);
char * fmt_ullong(unsigned long long int, struct FMT *);

char * fmt_hex(unsigned long int, struct FMT *);
char * fmt_double(double, struct FMT *);
char * fmt_ptr(void *, struct FMT *);

char * fmt_str(char *);

char * f(struct FMT *);

/*		PRINT FUNCTIONS		*/

void p_llong(long long int, struct FMT *);
void p_ullong(unsigned long long int, struct FMT *);

void p_hex(unsigned long int, struct FMT *);
void p_double(double, struct FMT *);
void p_str(char *, FILE *);
void p_ptr(void *, struct FMT *);

void p(struct FMT *);

/*		OTHER		*/

int parse(const char *, unsigned int *, struct FMT *);
unsigned int parse_uint(const char *, unsigned int *);

void pop_arg(struct FMT *, va_list);
int _print(const char *, va_list, FILE *);

unsigned int len(const char *);
int concat(char *, char *, unsigned int);

const char * type_to_str(unsigned int);

#ifdef _DEBUG
char * fmt_to_str(struct FMT *);
#endif
/*		Function Implementations	*/

const char hex_digits[] = "0123456789ABCDEF";

char * fmt_llong(long long int val, struct FMT * fmt) {
	char is_neg = val < 0, length = 1 + (is_neg | fmt->show_sign);
	
	if (is_neg) { val = -val; }

	unsigned long long int list = val;
	
	while (list /= 10) ++length;
	
	char * buf = malloc(sizeof(char) * (length + 1));
	
	if (is_neg) { buf[0] = '-'; fmt->show_sign = 1; }
	else if (fmt->show_sign) { buf[0] = '+'; }

	for (char i = length; i-- != fmt->show_sign; val /= 10) {
		buf[i] = '0' + (val % 10);
	}
	buf[length] = 0;
	fmt->str_length = length;

	#ifdef _DEBUG
		printf("Size Int: %d\n", fmt->str_length);
	#endif

	return buf;
}

char * fmt_ullong(unsigned long long int val, struct FMT * fmt) {
	long long unsigned int list = val;
	unsigned int length = 1;

	while ((list /= 10)) ++length;
	char * buf = malloc(sizeof(char) * (length + 1));
	for (unsigned int i = length; i--; val /= 10) {
		buf[i] = '0' + (val % 10);
	}
	buf[length] = 0;
	fmt->str_length = length;
	#ifdef _DEBUG
		printf("Size Unsigned: %d\n", fmt->str_length);
	#endif
	return buf;
}

char * fmt_hex(unsigned long int val, struct FMT * fmt) {
	unsigned long int list = val, length = 3;
	while (list >>= 4) ++length;
	char * buf = malloc(sizeof(char) * (length + 1));
	buf[0] = '0', buf[1] = 'x', buf[length] = 0;
	fmt->str_length = length;
	do {
		buf[--length] = hex_digits[val & 15];
		val >>= 4;
	} while (val);
	#ifdef _DEBUG
		printf("Size Hex: %d\n", fmt->str_length);
	#endif
	return buf;
} 

char * fmt_double(double val, struct FMT * fmt) { 
	char * buf = NULL;
    asprintf(&buf, "%f%n", val, &fmt->str_length);
	return buf;
}

char * fmt_ptr(void * val, struct FMT * fmt) {
	// printf("%p\n", val);
	if (val == NULL) {
		fmt->str_length = 6;
		return "(NULL)";
	} else {
		return fmt_hex(*((long int*)val), fmt);
	}
}

char * f(struct FMT * fmt) {
	switch (fmt->type) {
		case CHAR: {
						char * buf = malloc(2);
						buf[0] = fmt->i; buf[1] = 0;
						fmt->str_length = 1;
						return buf;
				   }
		case BOOL: {
					   if (fmt->i) {
						   fmt->str_length = 4;
						   return "true";
					   } else {
						   fmt->str_length = 5;
						   return "false";
					   }
				   }

		case INT:
		case LONG: 
		case LLONG: return fmt_llong(fmt->i, fmt);

		case UINT:
		case ULONG: 
		case ULLONG: return fmt_ullong((unsigned long long int)fmt->i, fmt);

		case HEX: return fmt_hex(fmt->i, fmt);

		case DOUBLE:
		case LDOUBLE: return fmt_double(fmt->d, fmt);

		case PTR: return fmt_ptr(fmt->ptr, fmt);
		case STR: {
						if (fmt->ptr != NULL) {
							while (*((char*)fmt->ptr + fmt->str_length++));
							fmt->str_length--;
						}
						#ifdef _DEBUG
							printf("Size Str: %d\n", fmt->str_length);
						#endif
						return fmt->ptr;
				  }
		default: {
					 println("\n[FMT Error]: '{s}' is not a valid type", type_to_str(fmt->type));
					 return fmt->ptr;
				 }
	}
}

void p_ullong(unsigned long long int val, struct FMT * fmt) {
	long long unsigned int reversed = 0;
	char length = 0;
	do {
		reversed = 10 * reversed + (val % 10); ++length;
	} while (val /= 10);

	while (length--) {
		putc('0' + (reversed % 10), fmt->fp);
		reversed /= 10;
	}
}

void p_llong(long long int val, struct FMT * fmt) {
	if (val < 0) {
		putc('-', fmt->fp);
		val = -val;
	} else if (fmt->show_sign) {
		putc('+', fmt->fp);
	}
	p_ullong(val, fmt);
}

void p_hex(unsigned long int val, struct FMT * fmt) {
	unsigned long int list = val;
	for (char length = 0; list >>= 4; ++length);
	p_str("0x", fmt->fp);
	do {
		putc(hex_digits[val & 15], fmt->fp);
	} while (val >>= 4);
}

void p_double(double val, struct FMT * fmt) {
	p_str(fmt_double(val, fmt), fmt->fp);
}

void p_str(char * val, FILE * fp) {
	if (val != NULL) {
		fputs(val, fp);
	}
}

void p_ptr(void * val, struct FMT * fmt) {
	if (val == NULL) {
		p_str("(NULL)", fmt->fp);
	} else {
		p_hex((unsigned long int)val, fmt);
	}

}

void p(struct FMT * fmt) {	
	switch (fmt->type) {
		case CHAR:		putc(fmt->i, fmt->fp); break;
		case BOOL:		p_str(fmt->i ? "true" : "false", fmt->fp); break;

		case INT:
		case LONG:
		case LLONG:		p_llong(fmt->i, fmt); break;
		
		case UINT:
		case ULONG:
		case ULLONG:	p_ullong(fmt->i, fmt); break;
		
		case HEX:		p_hex((unsigned int)fmt->i, fmt); break;
			
		case DOUBLE:
		case LDOUBLE:	p_double(fmt->d, fmt); break;
					
		case PTR:		p_ptr(fmt->ptr, fmt); break;
		case STR:		p_str(fmt->ptr, fmt->fp); break;
		default: println("\n[FMT Error]: '{s}' is not a valid type", type_to_str(fmt->type));
	}
}

unsigned int parse_uint(const char * format, unsigned int * index) {
	char c;
	for (unsigned int num = 0, i = *index - 1; (c = format[i]); i++) {
		if ('0' <= c && c <= '9') {
			num = 10 * num + c - '0';
			continue;
		}
		*index = i;
		return num;
	}
	return 0;
}

void pop_arg(struct FMT * fmt, va_list list) {
	
	switch (fmt->type) {
		case CHAR:		fmt->i = (char) va_arg(list, int); break;
		case BOOL:		fmt->i = (char) va_arg(list, int); break;
		
		case INT:		fmt->i =		va_arg(list, int); break;
		case LONG:		fmt->i =		va_arg(list, long int); break;
		case LLONG:		fmt->i =		va_arg(list, long long int); break;
		
		case UINT:		fmt->i =		va_arg(list, unsigned int); break;
		case ULONG:		fmt->i =		va_arg(list, unsigned long int); break;
		case ULLONG:	fmt->i =		va_arg(list, unsigned long long int); break;

		case HEX:		fmt->i =		va_arg(list, unsigned int); break;
		
		case DOUBLE:	fmt->d =		va_arg(list, double); break;
		case LDOUBLE:	fmt->d =		va_arg(list, long double); break;
		
		case PTR:
		case STR:		fmt->ptr =		va_arg(list, void *); break;
	}

}

int parse(const char * format, unsigned int * i, struct FMT * fmt) {
	char c;
	while ((c = format[(*i)++]) != '}') {
		if(c == EOF) {
			println("\n[FMT Error]: While parsing encountered end of string");
			return 0;
		}
		switch(c) {
			case 'c': fmt->type = CHAR; break; // char
			case 's': fmt->type = STR; break; // char *
			case 'i': fmt->type = INT; break; // int
			case 'u': fmt->type = UINT; break; // unsigned int
			case 'x': fmt->type = HEX; break; // hexadecimal
			case 'p': fmt->type = PTR; break; // memory address
			case 'd': fmt->type = DOUBLE; break; // double
			case 'D': fmt->type = LDOUBLE; break; // long double
			case 'b': fmt->type = BOOL; break; // boolean (char converted to 'true' or 'false')
			case 'r': fmt->repeat = 1; break; // repeat argument
			case 'q': fmt->var_length = 1; break; // use unsigned int variable for count
			case 'S': fmt->show_sign = 1; break; // show + before positive numbers
			case 'l': {
						int ret = parse(format, i, fmt);
						fmt->type++;
						return ret;
					  }
			case 'L': {
						int ret = parse(format, i, fmt);
						fmt->type += 2;
						return ret;
					  }
			case '!': {
						  int ret = parse(format, i, fmt);
						  fmt->stop = 1; 
						  return ret;
					  }
			case ':': {
						unsigned int length = *i;
						while (format[(length)++] != '}');
						length -= *i + 1;
						char * buf = malloc((length + 1) * sizeof(char));
						for (unsigned int j = 0; j < length; ++j) buf[j] = format[(*i)+j];
						*i += length + 1; 
						buf[length] = 0;
						fmt->d_length = length;
						fmt->delim = buf;
						return 1;
					  }
			case '*': { // list
						fmt->pointer = 1;
						fmt->count = 0;
						int ret = parse(format, i, fmt);
						if (!fmt->count || !fmt->var_length) {
							println("\n[FMT Error]: A pointer specifier must be followed by a lengthgth");
							return 0;
						} else if (fmt->count) fmt->pointer = fmt->count;
						return ret;
					}
			default:
					fmt->count = parse_uint(format, i);
		}
	}
	return 1;
}

int _print(const char * format, va_list list, FILE * fp) {
	
	unsigned int i = 0, stop_mode = 0;
	char c;
	while ((c = format[i++])) {
		if (stop_mode) {
			fputs(&format[i - 1], fp);
			break;
		} else if (c != '{') {
			putc(c, fp);
			continue;
		}

		struct FMT * fmt = init_fmt();
		if (!parse(format, &i, fmt)) {
			free(fmt);
			return 0;
		}
		fmt->fp = fp;

		if (fmt->var_length) {
			if (fmt->pointer) fmt->pointer = va_arg(list, unsigned int);
			else fmt->count = va_arg(list, unsigned int);
		}

		if (fmt->repeat) {
			pop_arg(fmt, list);
			for (unsigned int j = 0; j < fmt->count; ++j) {
				if (fmt->d_length && j) 
					p_str(fmt->delim, fmt->fp);
				p(fmt);
			}
		} else if (fmt->pointer) {
			fmt->ptr = va_arg(list, void *);
			for (unsigned int j = 0; j < fmt->pointer; ++j) {
				if (fmt->d_length && j)
					p_str(fmt->delim, fmt->fp);
				p(fmt);
			}
		} else {
			for (unsigned int j = 0; j < fmt->count; ++j) {
				if (fmt->d_length && j) 
					p_str(fmt->delim, fmt->fp);
				pop_arg(fmt, list);
				p(fmt);
			}
		}
		stop_mode = fmt->stop;
		// printf("\n%s\n\n", fmt_to_str(fmt));
		free(fmt);
	}
	return 1;
}

int print(const char * format, ...) {

	va_list list;
	va_start(list, format);
	int ret = _print(format, list, stdout);
	va_end(list);
	return ret;
}

int println(const char * format, ...) {

	va_list list;
	va_start(list, format);
	int ret = _print(format, list, stdout);
	va_end(list);
	putc(10, stdout);
	return ret;
}

int writef(FILE * out, const char * format, ...) {
	va_list list;
	va_start(list, format);
	int ret = _print(format, list, out);
	va_end(list);
	return ret;
}

char * format(const char * format, ...) {

	unsigned int size = len(format), buf_index = 0, stop_mode = 0;
	char * buf = malloc(size * sizeof(char));
	char c;
	va_list list;
	va_start(list, format);
	for (unsigned int i = 0; (c = format[i++]);) {
		if (buf_index >= size) {
			println("[FMT Error]: Attempted to writing past malloced buffer");
			free(buf);
			va_end(list);
			return 0;
		} else if (stop_mode || c != '{') {
			buf[buf_index++] = c;
			continue;
		}

		struct FMT * fmt = init_fmt();
		fmt->fp = stderr;
		if (!parse(format, &i, fmt)) {
			free(fmt);
			free(buf);
			va_end(list);
			return 0;
		}

		if (fmt->var_length) {
			if (fmt->pointer) fmt->pointer = va_arg(list, unsigned int);
			else fmt->count = va_arg(list, unsigned int);
		}
		
		char * src = 0;

		if (fmt->repeat) {
			pop_arg(fmt, list);
			for (unsigned int j = 0; j < fmt->count; ++j) {
				pop_arg(fmt, list);
				fmt->str_length = 0;
				src = f(fmt);
				size += fmt->str_length;
				if (fmt->d_length && j) {
					size += fmt->d_length;
					const unsigned int s_size = fmt->str_length + fmt->d_length;
					char * d_buf = malloc(sizeof(char) * (s_size + 1));
					concat(d_buf, fmt->delim, 0);
					concat(d_buf, src, fmt->d_length);
					src = d_buf;
					src[s_size] = 0;
				}
				buf = realloc(buf, sizeof(char) * size);
				if (buf == NULL)
					break;
				buf_index += concat(buf, src, buf_index);
			}
		} else if (fmt->pointer) {
			fmt->ptr = va_arg(list, void *);
			for (unsigned int j = 0; j < fmt->pointer; ++j) {
				pop_arg(fmt, list);
				fmt->str_length = 0;
				src = f(fmt);
				size += fmt->str_length;
				if (fmt->d_length && j) {
					size += fmt->d_length;
					const unsigned int s_size = fmt->str_length + fmt->d_length;
					char * d_buf = malloc(sizeof(char) * (s_size + 1));
					concat(d_buf, fmt->delim, 0);
					concat(d_buf, src, fmt->d_length);
					src = d_buf;
					src[s_size] = 0;
				}
				buf = realloc(buf, sizeof(char) * size);
				if (buf == NULL)
					break;
				buf_index += concat(buf, src, buf_index);
			}
		} else {
			for (unsigned int j = 0; j < fmt->count; ++j) {
				pop_arg(fmt, list);
				fmt->str_length = 0;
				src = f(fmt);
				size += fmt->str_length;
				if (fmt->d_length && j) {
					size += fmt->d_length;
					const unsigned int s_size = fmt->str_length + fmt->d_length;
					char * d_buf = malloc(sizeof(char) * (s_size + 1));
					concat(d_buf, fmt->delim, 0);
					concat(d_buf, src, fmt->d_length);
					src = d_buf;
					src[s_size] = 0;
				}
				buf = realloc(buf, sizeof(char) * size);
				if (buf == NULL) {
					break;
                }
				buf_index += concat(buf, src, buf_index);
			}
		}
		if (fmt->delim)
			free(fmt->delim);
		if (fmt->stop) {
			stop_mode = 1;
			int start = buf_index;
			while (format[++start]);
			size += start - buf_index;
			buf = realloc(buf, sizeof(char) * size);
		}
		free(fmt);
		if (buf == NULL) {
			free(buf);
			free(src);
			return NULL;
		}
	}
	buf[buf_index] = 0;
	#ifdef _DEBUG
		if (size != buf_index + 1) {
			println("{u} == {u}", size, buf_index);
			println("{s}", buf);
		}
	#endif
	va_end(list);
	return buf;

}


struct FMT * init_fmt() {

	struct FMT * fmt = calloc(1, sizeof(struct FMT));
	fmt->count = 1;

	return fmt;
}

unsigned int len(const char * str) {
	unsigned int length = 1;
	for (unsigned int i = 0; str[i]; ++i) {
		if (str[i] == '{') {
			while (str[++i] != '}');
		} else {
			++length;
		}
	}
	//printf("Format length: %u\n", len);
	return length;
}

int concat(char * dest, char * src, unsigned int dest_length) {
	unsigned int i = 0;
	if (src != NULL) {
		while (src[i]) {
			dest[dest_length++] = src[i++];
		}
	}
	dest[dest_length] = 0;
	return i;
}

const char * type_to_str(unsigned int type) {
	switch (type) {
		case INT: return "INT";
		case LONG: return "LONG INT"; 
		case LLONG: return "LONG LONG INT";
		case UINT: return "UNSIGNED INT";
		case ULONG: return "UNSIGNED LONG INT";
		case ULLONG: return "UNSIGNED LONG LONG INT";
		case CHAR: return "CHARACTHER";
		case BOOL: return "BOOLEAN";
		case HEX: return "HEXADECIMAL";
		case DOUBLE: return "DOUBLE";
		case LDOUBLE: return "LONG DOUBLE";
		case PTR: return "PTR";
		case STR: return "STRING";
		default: return format("UNDEFINED({u})", type);
	}
}

#ifdef _DEBUG
char * fmt_to_str(struct FMT * fmt) {

	const char * template = "<i='%li', d='%Lf', ptr='%p', count='%i', type='%s', hex='%d', repeat='%d', var_len='%d', pointer='%d'>";
	const char * type = type_to_str(fmt->type);
	unsigned int size = len(template) + len(type) + 1000;
	char * buf = calloc(size, sizeof(char));
	snprintf(buf, size, template, fmt->i, fmt->d, (int*)fmt->ptr, fmt->count, type, fmt->hex, fmt->repeat, fmt->var_len, fmt->ptr);
	return buf;
}
#endif

//////////////////////////////////////////////////////////////////////
// File: main.c
#include <system.h>
#include <io.h>
#define UCH (unsigned char *)

/* some convenient functions - as we don't ahve libc, we must do
   everything ourselves */

unsigned char *memcpy(unsigned char *dest, const unsigned char *src, int count)
{
  int i;
  for (i=0; i<count;i++) dest[i]=src[i];
  return dest;
}

unsigned char *memset(unsigned char *dest, unsigned char val, int count)
{
  int i;
  for (i=0; i<count;i++) dest[i]=val;
  return dest;
}

unsigned short *memsetw(unsigned short *dest, unsigned short val, int count)
{
  int i;
  for (i=0; i<count;i++) dest[i]=val;
  return dest;
}

int strlen(const char *str)
{
  int i;
  for (i=0;;i++) if (str[i] == '\0') return i;
}

/* We can use this for reading from the I/O ports to get data from
*  devices such as the keyboard. We are using what is called 'inline
*  assembly' in these routines to actually do the work. [XXX I still
*  have to add devices to the tutorial] */
unsigned char inportb (unsigned short _port)
{
    unsigned char rv;
    __asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (_port));
    return rv;
}

/* We can use this to write to I/O ports to send bytes to
*  devices. Again, we use some inline assembly for the stuff that
*  simply cannot be done in C */
void outportb (unsigned short _port, unsigned char _data)
{
    __asm__ __volatile__ ("outb %1, %0" : : "dN" (_port), "a" (_data));
}

typedef struct __instruction
{
	unsigned char operator;
	unsigned int operand;
} instruction_t;

typedef struct __program
{
	char name[32];
	instruction_t program[512];
	unsigned int pc;
} program_t;

void init_data(instruction_t * program, unsigned short * data, unsigned int * stack, unsigned int * sp, unsigned int * dp, unsigned int * pc)
{
	return;
	int i;
	for (i = 0; i < 64;   i++) {  program[i].operator = 0;
                                      program[i].operand  = 0;
				   }
	for (i = 0; i < 64;   i++)    stack  [i]          = 0;
	for (i = 0; i < 128;  i++)    data   [i]          = 0;
	                             *dp                  = 0;
                                     *pc                  = 0;
                                     *sp                  = 0;
}


int strcmp(char * a, char * b)
{
	int i;
	if (strlen(a) != strlen(b)) return 1;
	for (i = 0; i < strlen(a); i++)
	{
		if (a[i] != b[i]) return 2;
	}
	return 0;
}

/* This is a very simple main() function. All it does is print stuff
*  and then sit in an infinite loop. This will be like our 'idle'
*  loop */
void
cmain (unsigned long magic, unsigned long addr)
{
#define PUSH(a) stack[sp++] = a
#define POP()  stack[--sp]

	unsigned char opt;
	unsigned char tmp;
	program_t programs[3];
	unsigned short data[128];
	unsigned int dp = 0;
	unsigned int sp = 0;
	unsigned int pn = 0;
	unsigned int i;
	unsigned int stack[64];
	init_video();
	while (1)
	{
		//cls();
		opt = 0;
		puts (UCH "1. Load Program\n"
			  "2. Execute Program at addr\n");
		opt = getch();
		if (opt == '2')
		{
			char name[32];
			int np = 0;
			puts(UCH "Name: ");
			while ((tmp = getch()) != 0x0D)
			{
				putch(tmp);
				name[np++] = tmp;
			}
			putch(0x0A);
			for (i = 0; i < 3; i++)
			{
				if (strcmp(name, programs[i].name) == 0)
					pn = i;
			}
			//init_data(programs[0].program, data, stack, &sp, &dp, &programs[0].pc);
			//pc = programs[pn].pc;
			//cls();
			for (i = 0; i < 128; i++) data[i] = 0;
			puts(UCH "\nDebugging...\n");
			for (i = 0; i < programs[pn].pc; i++)
			{
				//putch(programs[pn].program[i].operator);
				//getch();
				switch(programs[pn].program[i].operator)
				{
					case '+':
						data[dp]++;
						break;
					case '-':
						data[dp]--;
						break;
					case '<':
						dp--;
						break;
					case '>':
						dp++;
						break;
					case '.':
						putch(data[dp]);
						break;
					case ',':
						data[dp] = getch();
						break;	
					case '[':
						puts(UCH "Entering loop\n");
						if (data[dp] == 0) i = programs[pn].program[i].operand;
						break;
					case ']':
						puts(UCH "Exiting loop\n");
						if (data[dp] != 0) i = programs[pn].program[i].operand;
						break;
					default: break;
				}
			}
			//puts(UCH "\n\n\nContinuing kernel execution\n");
		}
		else if (opt == '1')
		{
			int np = 0;
			puts(UCH "Slot #: ");
			pn = getch() - 48;
			puts(UCH "Name: ");
			while ((tmp = getch()) != 0x0D)
			{
				putch(tmp);
				programs[pn].name[np++] = tmp;
			}
			putch(0x0A);
			programs[pn].pc = 0;
			//init_data(program, data, stack, &sp, &dp, &programs[0].pc);
			sp = 0;
			puts(UCH "Bindings: {key, value} {=,+} {n,<} {m,>} {z,DEL}\n:");
			while ((tmp = getch()) != '~')
			{
				if (tmp == 0x0D) putch(0x0A);
				else if (tmp == 'z')
				{
					programs[pn].pc--;
					//pc--;
					putch('\b');
				}
				else
				{
					if (tmp == '=') tmp = '+';
					if (tmp == 'n') tmp = '<';
					if (tmp == 'm') tmp = '>';
					if (tmp == '[')
					{
						PUSH(programs[pn].pc);
					}
					if (tmp == ']')
					{
						programs[pn].program[programs[pn].pc].operand = POP();
						//programs[pn].program[programs[pn].pc].operand = programs[pn].pc;
						programs[pn].program[programs[pn].program[programs[pn].pc].operand].operand = programs[pn].pc
					}
					programs[pn].program[programs[pn].pc].operator = tmp;
					putch(programs[pn].program[programs[pn].pc].operator);
					programs[pn].pc++;
					//program[pc].operator = tmp;
					//putch(program[pc].operator);
					//pc++;
				}
			}
			puts(UCH "Done!\n\n");
			//programs[pn].pc = pc;
		}
	}
}




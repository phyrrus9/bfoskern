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

void readfloppy(void * buf)
{
	__asm__("mov $0x02, %ah\n\t"
		"mov $0x01, %al\n\t"
		"mov $0x00, %ch\n\t"
		"mov $0x01, %cl\n\t"
		"mov $0x00, %dh\n\t"
		"mov $0x00, %dl\n\t"
		"mov %0, %%es\n\t"
		"mov $0x00, %bs\n\t"
		"int 0x13" : "=m"(buf) );

	//__asm__ __volatile__ ("mov $2, %ah");
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
	unsigned int pc, dp;
	unsigned short data[128];
} program_t;


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

void execute_bf(program_t * programs, char * name)
{
	int pn = 0, i;
	puts(UCH "Looking for program named ");
	puts(UCH name);
	puts(UCH "\n");
	for (i = 0; i < 3; i++)
	{
		puts(UCH "Found program ");
		puts(UCH programs[i].name);
		puts(UCH "\n");
		if (strcmp(name, programs[i].name) == 0)
		pn = i;
	}
	if (pn == 0)
	{
		puts(UCH "Program not found!\n");
		return;
	}
	//cls();
	for (i = 0; i < 128; i++) programs[pn].data[i] = 0;
	programs[pn].dp = 0;
	puts(UCH "\nDebugging...\n");
	for (i = 0; i < programs[pn].pc; i++)
	{
		//putch(programs[pn].program[i].operator);
		//getch();
		switch(programs[pn].program[i].operator)
		{
			case '+':
				programs[pn].data[programs[pn].dp]++;
				break;
			case '-':
				programs[pn].data[programs[pn].dp]--;
				break;
			case '<':
				programs[pn].dp--;
				break;
			case '>':
				programs[pn].dp++;
				break;
			case '.':
				putch(programs[pn].data[programs[pn].dp]);
				break;	
			case ',':
				programs[pn].data[programs[pn].dp] = getch();
				break;	
			case '[':
				puts(UCH "Entering loop\n");
				if (programs[pn].data[programs[pn].dp] == 0) i = programs[pn].program[i].operand;
				break;
			case ']':
				puts(UCH "Exiting loop\n");
				if (programs[pn].data[programs[pn].dp] != 0) i = programs[pn].program[i].operand;
				break;
			default: break;
		}
	}
	//puts(UCH "\n\n\nContinuing kernel execution\n");
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
	unsigned int sp = 0;
	unsigned int pn = 0;
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
			name[np] = 0;
			putch(0x0A);
			execute_bf(programs, name);
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
						programs[pn].program[programs[pn].program[programs[pn].pc].operand].operand = programs[pn].pc;
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




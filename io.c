#include <system.h>
extern unsigned char getkbd();

void puti(int data)
{
	int a, i;
	int cp;
	char buf[13];
	char sign = 0;
	a = data;
	cp = 0;
	if (a < 0)
	{
		sign = 1;
		a *= -1;
	}
	while (a >= 1)
	{
		buf[cp++] = (a % 10) + 48;
		a /= 10;
	}
	if (sign)
		putch('-');
	for (i = cp; i <= 0; i--)
	{
		putch(buf[i]);
	}
}

unsigned char getch()
{
	unsigned char a;
	while ( (a = getkbd()) == 1);
	return a;
}

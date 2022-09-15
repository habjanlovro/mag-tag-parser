#include <stdio.h>
#include <unistd.h>

char a = 'a';
char b = 'b';
char c = 'c';
char buf[10] = { '1' };

int main() {

	buf[0] = a;
	buf[1] = b;
	buf[2] = c;
	write(1, buf, 3);
}
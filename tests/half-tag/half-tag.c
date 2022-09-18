#include <stdio.h>
#include <unistd.h>

char a = 'a';
char b = 'b';
char c = 'c';
char buf[5] = { '1', '2', '3', '4', '\n' };

int main() {

	buf[0] = a;
	buf[1] = b;
	buf[2] = c;
	write(1, buf, 5);
}

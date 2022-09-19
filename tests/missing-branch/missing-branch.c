#include <stdio.h>

volatile int a = 1;
volatile int b = 5;

int main() {
#ifdef EQUALITY
	if (a == 1) {
#else
	if (a > 1) {
#endif
		b++;
	}
	printf("%d\n", b);
	
	return 0;
}

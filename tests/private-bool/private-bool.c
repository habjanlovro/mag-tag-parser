#include "stdio.h"

int a = 1;
int b = 1;

int main() {
	if (a) {
		b = 0;
	} else {
		b = 1;
	}

	printf("%d", b);
	printf("\n");

	return 0;
}

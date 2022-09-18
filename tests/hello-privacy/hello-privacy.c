#include <unistd.h>

char *s = "Hello, privacy!\n";

int main() {
	write(1, s, 16);
	return 0;
}

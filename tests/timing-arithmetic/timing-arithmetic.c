volatile int a = 1;
volatile int b = 20;

int main() {
	for (int i = 0; i < 100000000; i++) {
		a + i - b;
	}
	return 0;
}
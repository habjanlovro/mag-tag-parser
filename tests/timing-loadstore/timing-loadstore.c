volatile int a = 50;
volatile int b = 30;

int main() {
	for (int i = 0; i < 100000000; i++) {
		int tmp = a;
		a = b;
		b = tmp;
	}
	return 0;
}
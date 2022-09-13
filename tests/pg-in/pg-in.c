#include <stdio.h>
#include <string.h>

char password[9] = "12345678";

int main() {
	printf("Please enter password: ");
	scanf("%s", password);
	if (strlen(password) >= 8) {
		printf("OK length\n");
	} else {
		printf("NOK length\n");
	}
	return 0;
}
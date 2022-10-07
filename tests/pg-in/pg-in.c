#include <stdio.h>
#include <string.h>

char password[9] = "12345678";
char input_password[9] = "";

int main() {
	printf("Please enter password:\n");
	scanf("%s", input_password);
	printf("Password: %s\n", input_password);
	return 0;
}
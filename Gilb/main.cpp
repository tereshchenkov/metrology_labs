#include <stdio.h>
#include <locale.h>
#include "constants.h"
#include "process.h"

int main(void)
{
	setlocale(LC_ALL, "Russian");
	char buffer[MAX_STR_LENGTH];
	init_buffer();
	FILE* file;
	fopen_s(&file, "prog.pas", "r");
	while (!feof(file))
	{
		fgets(buffer, MAX_STR_LENGTH, file);
		add_to_buffer(buffer);
	}
	fclose(file);

	process_program();
	getchar();
	return 0;
}

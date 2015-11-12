#include <stdio.h>
#include "constants.h"
#include "process.h"
#include <locale.h>

int main(void)
{
	setlocale(LC_ALL, "Russian");
	char buffer[MAX_STR_LENGTH];
	init_buffer();
	FILE* file;
	fopen_s(&file, "prog.dpr", "r");
	while (!feof(file))
	{
		// Читаем файл в глобальный буфер
		fgets(buffer, MAX_STR_LENGTH, file);
		add_to_buffer(buffer);
	}
	fclose(file);
	
	// Обработка текста программы
	int answer = process_program();
	printf("Итоговая метрика Мак-Кейба по всем функциям: %d\n", answer);
	getchar();
	return 0;
}

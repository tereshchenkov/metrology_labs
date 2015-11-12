// process.cpp - содержит реализацию функций для обработки текста

#include <stdio.h>
#include <string.h>
#include "process.h"
#include "graph.h"
#include "constants.h"

int include_level;
char program_buf[MAX_PROGRAM_LENGTH];
int current_position = 0;
int global_McCabe;
int local_McCabe;
int reached_end;
int else_exists;
int until_exists;
int processing_function = NO;
char current_function[MAX_WORD_LENGTH] = {0};
char words[WORD_COUNT][MAX_WORD_LENGTH] = {"procedure", "function", "begin", "end", "if",
										   "else", "case", "while", "repeat", "until", "for"};

void init_buffer()
{
	program_buf[0] = '\0';
}

char* get_buffer()
{
	return program_buf;
}

void add_to_buffer(char new_part[])
{
	strcat_s(program_buf, new_part);
}

int process_program()
{
	global_McCabe = 1;
	reached_end = NO;
	while (reached_end == NO)
		search_include();
	return global_McCabe;
}

void process_function()
{
	local_McCabe = 1;
	include_level = 1;
	init_graph();
	while (include_level > 0)
		search_include();
	if (current_function[0] == '\0')
		strcpy_s(current_function, "main");
	int end_vertex = add_vertex();
	add_edge(get_current_vertex(), end_vertex);
	printf("Результаты по функции %s:\n", current_function);
	printf("Граф: ");
	print_graph();
	printf("Метрика Мак-Кейба, расчитанная для функции: %d\n\n", local_McCabe);
	current_function[0] = '\0';
	processing_function = NO;
	global_McCabe += local_McCabe;
	global_McCabe--;
}

void add_function_name(char name[])
{
	strcpy_s(current_function, name);
}

int init_include()
{
	local_McCabe++;
	int start_vertex = add_vertex();
	add_edge(get_current_vertex(), start_vertex);
	set_current_vertex(start_vertex);
	return start_vertex;
}

void process_if()
{
	int start_vertex = init_include();
	search_include();
	int end_vertex = add_vertex();
	// Добавление первой ветки оператора if
	add_edge(get_current_vertex(), end_vertex);
	set_current_vertex(start_vertex);
	int temp_position = current_position;
	char temp_buffer[MAX_WORD_LENGTH];
	if ((else_exists == YES) || (get_next_word(temp_buffer) == ELSE))
	{
		// Поиск вложенностей во второй ветке оператора if
		search_include();
		else_exists = NO;
	}
	else
		current_position = temp_position;
	// Добавление второй ветки оператора if
	add_edge(get_current_vertex(), end_vertex);
	set_current_vertex(end_vertex);
}

void process_case()
{
	int start_vertex = init_include();
	int end_vertex = add_vertex();
	int current_level = include_level++;
	int was_else = NO;
	do
	{
		search_include();
		if ((current_level < include_level) && (else_exists == NO))
		{
			// Добавление очередной ветки
			local_McCabe++;
			add_edge(get_current_vertex(), end_vertex);
		}
		else
		{
			if (else_exists == YES)
			{
				// Присутствует ветка else
				was_else = YES;
				else_exists = NO;
				local_McCabe--;
			}
		}
	}
	while (current_level < include_level);
	// Если ветки else не было, то добавляем огибающую ветку
	if (was_else == NO)
		add_edge(start_vertex, end_vertex);
	local_McCabe--;
	set_current_vertex(end_vertex);
}

void process_for_while()
{
	int start_vertex = init_include();
	search_include();
	int new_vertex = add_vertex();
	add_edge(get_current_vertex(), new_vertex);
	add_edge(new_vertex, start_vertex);
	int end_vertex = add_vertex();
	add_edge(start_vertex, end_vertex);
	set_current_vertex(end_vertex);
}

void process_repeat()
{
	int start_vertex = init_include();
	until_exists = NO;
	while (until_exists == NO)
		search_include();
	int new_vertex = add_vertex();
	add_edge(get_current_vertex(), new_vertex);
	add_edge(new_vertex, start_vertex);
	int end_vertex = add_vertex();
	add_edge(new_vertex, end_vertex);
	set_current_vertex(end_vertex);
}

int get_next_word(char buffer[])
{
	int state = SPACE;
	int found_word = NO;
	int word_position;
	int word_kind;
	char word_buf[MAX_WORD_LENGTH];
	while (found_word == NO)
	{
		char symbol = program_buf[current_position++];
		// Приводим все к нижнему регистру
		if (symbol >= 'A' && symbol <= 'Z')
			symbol |= 0x20;
		// Реализация конечного автомата
		switch (state)
		{
		case SPACE:
			if (valid_symbol(symbol))
			{
				word_position = 0;
				word_buf[word_position++] = symbol;
				state = WORD;
			}
			else
				switch (symbol)
				{
				case ';':
					return OPERATOR_END;
				break;
				case '.':
					// Отличить задание диапазона значений от конца программы
					if (program_buf[current_position - 1] != '.')
						return PROGRAM_END;
				break;
				case '{':
					state = COM;
				break;
				case '/':
					if (program_buf[current_position] == '/')
						state = ONE_STR_COM;
				break;
				case '\'':
					state = STR;
				}
		break;
		case WORD:
			if (valid_symbol(symbol))
				word_buf[word_position++] = symbol;
			else
			{
				// Дополнительная обработка для конца оператора и конца программы
				if (symbol == ';' || symbol == '.')
					current_position--;
				word_buf[word_position] = '\0';
				word_kind = compare_word(word_buf);
				// Дополнительная проверка
				if (word_kind == END)
				{
					if (program_buf[current_position] == ';')
					{
						current_position++;
					}
					if (program_buf[current_position] == '.')
					{
						word_kind = PROGRAM_END;
						current_position++;
					}
				}
				strcpy_s(buffer, MAX_WORD_LENGTH, word_buf);
				found_word = YES;
			}
		break;
		case STR:
			if (symbol == '\'')
				state = SPACE;
		break;
		case ONE_STR_COM:
			if (symbol == '\n')
				state = SPACE;
		break;
		case COM:
			if (symbol == '}')
				state = SPACE;
		break;
		}
	}
	return word_kind;
}

void search_include()
{
	int word_type;
	do
	{
		char word_value[MAX_WORD_LENGTH];
		word_type = get_next_word(word_value);
		switch (word_type)
		{
		case PROCEDURE:
		case FUNCTION:
			// Получить и запомнить название функции
			get_next_word(word_value);
			add_function_name(word_value);
		break;
		case IF:
			process_if();
		break;
		case CASE:
			process_case();
		break;
		case ELSE:
			else_exists = YES;
		break;
		case FOR:
		case WHILE:
			process_for_while();
		break;
		case REPEAT:
			process_repeat();
		break;
		case UNTIL:
			until_exists = YES;
		break;
		case BEGIN:
			if (processing_function == YES)
			{
				int current_level = include_level++;
				// Ищем вложенности до возвращения из составного оператора
				while (include_level > current_level)
					search_include();
			}
			else
			{
				processing_function = YES;
				process_function();
			}
		break;
		case PROGRAM_END:
			reached_end = YES;
		case END:
			include_level--;
		break;
		}
	}
	while (word_type == EMPTY);
}

int compare_word(char buffer[])
{
	int i;
	for (i = 0; i < WORD_COUNT; i++)
		if (!strcmp(words[i], buffer))
			return i;
	return EMPTY;
}

int valid_symbol(char symbol)
{
	if ((symbol >= 'a' && symbol <= 'z') ||
		(symbol >= '0' && symbol <= '9') ||
		(symbol == '_'))
		return YES;
	else
		return NO;
}

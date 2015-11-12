// process.cpp - содержит реализацию функций для обработки текста

#include <stdio.h>
#include <string.h>
#include "process.h"
#include "constants.h"

int if_count;
int operators_count;
int max_if_depth;
int local_if_count;
int local_operators_count;
int local_max_if_depth;
int temp_if_depth;
int include_level;
char program_buf[MAX_PROGRAM_LENGTH];
int current_position = 0;
int reached_end;
int else_exists;
int until_exists;
int prev_result;
int processing_function = NO;
char current_function[MAX_WORD_LENGTH] = {0};
char words[WORDS_COUNT][MAX_WORD_LENGTH] = {"procedure", "function", "begin", "end", "if",
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

void process_program()
{
	if_count = 0;
	operators_count = 0;
	max_if_depth = 0;
	reached_end = NO;
	while (reached_end == NO)
		search_include();
	printf("Итоги по метрике Джилба:\n");
	printf("\tКоличество операторов if: %d\n", if_count);
	printf("\tОбщее число операторов: %d\n", operators_count);
	printf("\tНасыщенность кода операторами if: %.3f\n", (float)if_count / operators_count);
	printf("\tМаксимальная глубина вложенности if: %d\n", max_if_depth);
}

void end_process_function()
{
	printf("Результаты по функции %s:\n", current_function);
	printf("\tКоличество операторов if: %d\n", local_if_count);
	printf("\tОбщее число операторов: %d\n", local_operators_count);
	printf("\tНасыщенность кода операторами if: %.3f\n", (float)local_if_count / local_operators_count);
	printf("\tМаксимальная глубина вложенности if: %d\n\n", local_max_if_depth);
	if_count += local_if_count;
	operators_count += local_operators_count;
	if (local_max_if_depth > max_if_depth)
		max_if_depth = local_max_if_depth;
	current_function[0] = '\0';
	processing_function = NO;
}

void process_function()
{
	include_level = 1;
	local_if_count = 0;
	local_operators_count = 0;
	local_max_if_depth = 0;
	temp_if_depth = 0;
	while (include_level > 0)
		search_include();
	if (current_function[0] == '\0')
		strcpy_s(current_function, "main");
	end_process_function();
}

void add_function_name(char name[])
{
	strcpy_s(current_function, name);
}

void process_if()
{
	temp_if_depth++;
	local_operators_count++;
	local_if_count++;
	else_exists = NO;
	search_include();
	char temp_buffer[MAX_WORD_LENGTH];
	int temp_position = current_position;
	int temp_result = prev_result;
	if ((else_exists == YES) || (get_next_word(temp_buffer) == ELSE))
	{
		local_operators_count++;
		search_include();
		else_exists = NO;
	}
	else
	{
		current_position = temp_position;
		prev_result = temp_result;
	}
	if (temp_if_depth > local_max_if_depth)
		local_max_if_depth = temp_if_depth;
	temp_if_depth--;
}

void process_case()
{
	include_level++;
	local_operators_count++;
}

void process_for_while()
{
	local_operators_count++;
}

void process_repeat()
{
	local_operators_count++;
}

void process_begin()
{
	local_operators_count++;
	int current_level = include_level++;
	while (include_level > current_level)
		search_include();
}

void process_operator_end()
{
	if (processing_function == YES)
		local_operators_count++;
}

void process_end()
{
	if (processing_function == YES)
		if ((prev_result != OPERATOR_END) &&
			(prev_result != BEGIN) &&
			(prev_result != IF))
			local_operators_count++;
	include_level--;
}

int get_next_word(char buffer[])
{
	int state = SPACE;
	int found = NO;
	int word_position;
	int word_kind;
	char word_buf[MAX_WORD_LENGTH];
	while (found == NO)
	{
		char symbol = program_buf[current_position++];
		if (symbol >= 'A' && symbol <= 'Z')
			symbol |= 0x20;
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
				if (symbol == ';' || symbol == '.')
					current_position--;
				word_buf[word_position] = '\0';
				word_kind = compare_word(word_buf);
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
				found = YES;
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
	int next_word;
	do
	{
		char word[MAX_WORD_LENGTH];
		next_word = get_next_word(word);
		switch (next_word)
		{
		case PROCEDURE:
		case FUNCTION:
			get_next_word(word);
			add_function_name(word);
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
				process_begin();
			else
			{
				processing_function = YES;
				process_function();
			}
		break;
		case PROGRAM_END:
			reached_end = YES;
		case END:
			process_end();
		break;
		case OPERATOR_END:
			process_operator_end();
		break;
		}
		prev_result = next_word;
	}
	while (next_word == EMPTY);
}

int compare_word(char buffer[])
{
	int i;
	for (i = 0; i < WORDS_COUNT; i++)
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

// process.h - содержит объявления функции для обработки текста

void init_buffer();
char* get_buffer();
void add_to_buffer(char[]);
void process_program();
void process_function();
void process_if();
void process_case();
void process_for_while();
void process_repeat();
int get_next_word(char*);
void search_include();
int valid_symbol(char symbol);
int compare_word(char[]);
void add_function_name(char[]);

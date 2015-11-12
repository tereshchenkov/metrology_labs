// graph.h - содержит объявления функций для работы с графами

#define MAX_EDGE_COUNT 200

struct edge {
	int vertex_out;
	int vertex_in;
};

int get_current_edge();
int get_current_vertex();
void set_current_edge(int);
void set_current_vertex(int);

int add_vertex();
void add_edge(int, int);

void init_graph();
void print_graph();

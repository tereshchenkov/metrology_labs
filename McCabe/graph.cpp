// graph.cpp - содержит реализацию функций для работы с графами

#include "graph.h"
#include <stdio.h>

int current_vertex;
int current_edge;
int vertex_count;
struct edge graph[MAX_EDGE_COUNT];

int get_current_edge()
{
	return current_edge;
}

int get_current_vertex()
{
	return current_vertex;
}

void set_current_edge(int new_edge)
{
	current_edge = new_edge;
}

void set_current_vertex(int new_vertex)
{
	current_vertex = new_vertex;
}

int add_vertex()
{
	return (++vertex_count);
}

void add_edge(int vertex_out, int vertex_in)
{
	struct edge new_edge = {vertex_out, vertex_in};
	graph[current_edge++] = new_edge;	
}

void init_graph()
{
	current_vertex = 0;
	current_edge = 0;
	vertex_count = 0;
}

void print_graph()
{
	int i;
	for (i = 0; i < current_edge - 1; i++)
		printf("%d -> %d, ", graph[i].vertex_out, graph[i].vertex_in);
	printf("%d -> %d ", graph[i].vertex_out, graph[i].vertex_in);
	printf("\n");
}

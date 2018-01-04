#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Constants

#define BUFFER 256
#define TABLE_SIZE 1024
const char token[2] = " ";


// Structures

typedef struct Var
{
	char* name;
	int value;
} Var;

typedef struct Gate
{
	char* name;
	Var** inputs;
	Var** outputs;
	Var** selectors;
	int num_inputs;
	int num_selectors;
	int num_outputs;
	char** graycode;
} Gate;

typedef struct Node
{
	struct Node* next;
	Gate* gate;
	Var* var;
} Node;

typedef struct LL
{
	Node* head;
	Node* tail;
	int length;
} LL;


// Globals

LL* gates;
Node* vars[TABLE_SIZE];
Var** input_vars;
int input_count;
Var** output_vars;
int output_count;


//Declarations

Node* new_gate_node(Gate*);
Node* new_var_node(Var*);
void insert_gate(Gate*);
Var* new_var(char*);
Gate* new_gate(char*, int, int);
void parse_line(char*);
void read_file(FILE*);
void initialize_LL();
char* copy_string(char*);
void make_gate(char*, int, int);
unsigned int get_hash(char*);
void run_circuit();
char* copy_string_buffered(char*);
char** generate_graycode(char**, int, int);
char** get_graycode(int);
void simulate_circuit();

//Helper Functions

void free_graycode(char** code, int size)
{
	int i;
	for (i = 0; i < size; i++)
	{
		free(code[i]);
	}
	free(code);
}

void free_var(Var* var)
{
	free(var->name);
	free(var);
}

void free_gate(Gate* gate)
{
	free(gate->inputs);
	free(gate->outputs);
	free(gate->selectors);
	if (gate->graycode != NULL)
	{
		if (gate->name[0] == 'M') //Multiplexer
		{
			free_graycode(gate->graycode, gate->num_inputs);
		}
		else //Decoder
		{
			free_graycode(gate->graycode, gate->num_outputs);
		}
	}
	free(gate);
}

void free_node(Node* node)
{
	if (node->gate != NULL) free_gate(node->gate);
	if (node->var != NULL) free_var(node->var);
	free(node);
}

void free_LL(LL* list)
{
	Node* curr = list->head;
	Node* prev = NULL;
	while (curr != NULL)
	{
		prev = curr;
		curr = curr->next;
		free_node(prev);
	}
	free(list);
}

void free_everything()
{
	//does not correctly free vars, or gate graycode;
	free_LL(gates);
}

int char_to_digit(char c)
{
	return c - '0';
}

void print_results()
{
	int i;
	for (i = 0; i < input_count; i++)
	{
		printf("%d ", input_vars[i]->value);
	}
	for (i = 0; i < output_count - 1; i++)
	{
		printf("%d ", output_vars[i]->value);
	}
	printf("%d\n", output_vars[i]->value);
}

void print_current_vars()
{
	printf("Printing Main Variables:\n");
	int i;
	printf("\tInputs:\n");
	for (i = 0; i < input_count; i++)
	{
		printf("\t\t[%p] %s: %d\n", input_vars[i], input_vars[i]->name, input_vars[i]->value);
	}
	printf("\tOutputs:\n");
	for (i = 0; i < output_count; i++)
	{
		printf("\t\t[%p] %s: %d\n", output_vars[i], output_vars[i]->name, output_vars[i]->value);
	}
	printf("\n");
}

char digit_to_char(int digit)
{
	return '0' + digit;
}

int two_to_the(int num)
{
	return 1 << num;
}

void print_graycode(char** code, int len)
{
	int i;
	for (i = 0; i < len; i++)
	{
		printf("%s\n", code[i]);
	}
}

FILE* get_file_pointer(char* filename)
{
	FILE* fp = NULL;
	fp = fopen(filename, "r");
	return fp;
}

int lg(int num)
{
	int power = 0;
	while (1)
	{
		if (num <= 1 << power)
		{
			return power;
			break;
		}
		power++;
	}
	return -1;
}

void print_gate_abbrev(Gate* gate)
{
	printf("GATE [%p]: %s | IN: ", (void*) gate, gate->name);
	int i;
	for (i = 0; i < gate->num_inputs; i++)
	{
		printf("(%s: %d) ", gate->inputs[i]->name, gate->inputs[i]->value);
	}
	if (gate->selectors != NULL)
	{
		printf("| SEL: ");
		for (i = 0; i < gate->num_selectors; i++)
		{
			printf("(%s: %d) ", gate->selectors[i]->name, gate->selectors[i]->value);
		}
	}
	printf("| OUT: ");
	for (i = 0; i < gate->num_outputs; i++)
	{
		printf("(%s: %d)", gate->outputs[i]->name, gate->outputs[i]->value);
	}
	printf("\n");
}

void print_gate(Gate* gate)
{
	printf("GATE %p:\n", (void*) gate);
	printf("\tName = %s\n", gate->name);
	int i;
	printf("\tInputs:\n");
	for (i = 0; i < gate->num_inputs; i++)
	{
		printf("\t\t[%p] %s: %d\n", (void*) gate->inputs[i], gate->inputs[i]->name, gate->inputs[i]->value);
	}
	if (gate->selectors != NULL)
	{
		printf("\n\tSelectors:\n");
		for (i = 0; i < gate->num_selectors; i++)
		{
			printf("\t\t[%p] %s: %d\n", (void*) gate->selectors[i], gate->selectors[i]->name, gate->selectors[i]->value);
		}
	}
	printf("\n\tOutputs:\n");
	for (i = 0; i < gate->num_outputs; i++)
	{
		printf("\t\t[%p] %s: %d\n", (void*) gate->outputs[i], gate->outputs[i]->name, gate->outputs[i]->value);
	}
	printf("\n");
}

void print_LL()
{
	Node* curr = gates->head;
	while (curr != NULL)
	{
		print_gate(curr->gate);
		curr = curr->next;
	}
	return;
}
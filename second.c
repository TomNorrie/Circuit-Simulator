#include "second.h"

int main(int argc, char** argv)
{
	initialize_LL();
	read_file(get_file_pointer(argv[1]));

	compute_circuit_edges();
	sort_circuit();

	simulate_circuit();

	free_everything();
	return 0;
}


//Functions

void compute_circuit_edges()
{
	Node* A = gates->head;
	Node* B;


	while (A != NULL)
	{
		B = gates->head;
		while (B != NULL)
		{
			if (precedes(A,B))
			{
				add_edge(A,B);
			}
			B = B->next;
		}
		A = A->next;
	}
	// printf("edges computed\n\n");
}

void add_edge(Node* A, Node* B)
{
	add_to_head(copy_node(B), A->edges);
}

int precedes(Node* A, Node* B)
{
	int i,j;

	for (i = 0; i < A->gate->num_outputs; i++)
	{
		for (j = 0; j < B->gate->num_inputs; j++)
		{
			if (A->gate->outputs[i] == B->gate->inputs[j])
			{
				// print_gate_abbrev(A->gate);
				// printf("must precede\n");
				// print_gate_abbrev(B->gate);
				// printf("\n");
				return 1;
			}
		}
		for (j = 0; j < B->gate->num_selectors; j++)
		{
			if (A->gate->outputs[i] == B->gate->selectors[j])
			{
			// 	print_gate_abbrev(A->gate);
			// 	printf("must precede\n");
			// 	print_gate_abbrev(B->gate);
				// printf("\n");
				return 1;
			}
		}
	}
	return 0;
}

void sort_circuit()
{
	// printf("num gates pre-sort = %d\n", gates->length);
	LL* sorted = new_LL();
	Node* curr = gates->head;

	// print_gates();

	while (curr != NULL)
	{
		// printf("Checking ");
		// print_gate_name(curr->gate);
		// printf("\n");
		if (curr->visited == 0) visit(curr, sorted);
		curr = curr->next;
	}

	LL* orig = gates;
	gates = sorted;

	// printf("num gates post-sort = %d\n", gates->length);

	free(orig);
}

void visit(Node* node, LL* sorted)
{
	// printf("Visiting ");
	// print_gate_abbrev(node->gate);
	if (node->visited)
	{
		// printf("\talready visited\n\n");
		return;
	}
	// printf("\n");

	Node* curr = node->edges->head;

	while (curr != NULL)
	{
		visit(curr, sorted);
		curr = curr->next;
	}

	node->visited = 1;
	add_to_head(copy_node(node), sorted);
}

void add_to_head(Node* n, LL* list)
{
	n->next = list->head;
	list->length++;
	list->head = n;
}

void add_to_tail(Node* n, LL* list)
{
	if (list->head == NULL)
	{
		list->head = n;
		list->tail = n;
	}
	else
	{
		list->tail->next = n;
		list->tail = n;
	}
	list->length++;
}

LL* new_LL()
{
	LL* list = (LL*) malloc(1 * sizeof(LL));
	list->length = 0;
	return list;
}

Node* new_gate_node(Gate* gate)
{
	Node* n = (Node*) malloc(1*sizeof(Node));
	n->next = NULL;
	n->gate = gate;
	n->visited = 0;
	n->edges = new_LL();
	return n;
}

Node* new_var_node(Var* var)
{
	Node* n = (Node*) malloc(1*sizeof(Node));
	n->next = NULL;
	n->var = var;
	n->visited = -1;
	n->edges = NULL;
	return n;
}

Node* copy_node(Node* A)
{
	Node* B = (Node*) malloc(1 * sizeof(Node));
	B->next = A->next;
	B->gate = A->gate;
	B->var = A->var;
	B->visited = A->visited;
	B->edges = A->edges;
	return B;
}


// Functions from First

void simulate_circuit()
{
	int runs = two_to_the(input_count);
	char** graycode = get_graycode(runs);

	int i,j;

	for (i = 0; i < runs; i++)
	{
		// printf("\nRUN #%d/%d:\n", i, runs-1);
		for (j = 0; j < input_count; j++)
		{
			int digit = char_to_digit(graycode[i][j]);
			input_vars[j]->value = digit;
			// printf("\tSetting var %s to %d\n", input_vars[j]->name, digit);

		}
		run_circuit();
		print_results();
		// print_current_vars();
	}

	free_graycode(graycode, runs);
}

void parse_line(char* line)
{
	// printf("Parsing Line: %s\n", line);
	char first_letter = line[0];

	switch(first_letter)
	{
		case 'A': //and
		{
			make_gate(line, 2, 1);
			break;
		}
			
		case 'D': //decoder
		{
			char* word;
			word = strtok(line, token);
			word = strtok(NULL, token);

			int num_inputs;
			sscanf(word, "%d", &num_inputs);

			int num_outputs = 1 << num_inputs;

			Gate* gate = new_gate("DECODER", num_inputs, num_outputs);

			int i;
			for (i = 0; i < num_inputs; i++)
			{
				word = strtok(NULL, token);
				gate->inputs[i] = new_var(word);
			}
			for (i = 0; i < num_outputs; i++)
			{
				word = strtok(NULL, token);
				gate->outputs[i] = new_var(word);
			}

			gate->graycode = get_graycode(gate->num_outputs);

			insert_gate(gate);

			break;
		}	
		case 'I': //inputvar
		{
			char* word;
			word = strtok(line, token);
			word = strtok(NULL, token);
			int i;
			sscanf(word, "%d", &input_count);
			// printf("input_count = %d\n", input_count);
			input_vars = (Var**) malloc(input_count * sizeof(Var*));
			for (i = 0; i < input_count; i++)
			{
				word = strtok(NULL, token);
				input_vars[i] = new_var(word);
			}
			break;
		}
		case 'M': //multiplexer
		{
			char* word;
			word = strtok(line, token);
			word = strtok(NULL, token);

			int num_inputs;
			sscanf(word, "%d", &num_inputs);

			int num_outputs = 1;

			int num_selectors = lg(num_inputs);

			Gate* gate = new_gate("MULTIPLEXER", num_inputs, num_outputs);

			gate->num_selectors = num_selectors;
			gate->selectors = (Var**) malloc(num_selectors * sizeof(Var*));

			int i;
			for (i = 0; i < num_inputs; i++)
			{
				word = strtok(NULL, token);
				gate->inputs[i] = new_var(word);
			}
			for (i = 0; i < num_selectors; i++)
			{
				word = strtok(NULL, token);
				gate->selectors[i] = new_var(word);
			}
			word = strtok(NULL, token);
			gate->outputs[0] = new_var(word);

			gate->graycode = get_graycode(gate->num_inputs);

			insert_gate(gate);

			break;
		}
		case 'N':
		{
			switch(line[2])
			{
				case 'N': //NAND
				{
					make_gate(line, 2, 1);
					break;
				}
				case 'R': //NOR
				{
					make_gate(line, 2, 1);
					break;
				}
				case 'T': //NOT
				{
					make_gate(line, 1, 1);
					break;
				}
			}
			break;
		}
		case 'O':
		{
			if (line[1] == 'R') //or
			{
				make_gate(line, 2, 1);
			}
			else //outputvar
			{
				char* word;
				word = strtok(line, token);
				word = strtok(NULL, token);
				int i;
				sscanf(word, "%d", &output_count);
				output_vars = (Var**) malloc(output_count * sizeof(Var*));
				for (i = 0; i < output_count; i++)
				{
					word = strtok(NULL, token);
					output_vars[i] = new_var(word);
				}
			}
			break;
		}
		case 'X': //xor
		{
			make_gate(line, 2, 1);
			break;
		}
		default :
		{
			break;
		}
	}
}

void make_gate(char* line, int num_inputs, int num_outputs)
{
	char* word;

	word = strtok(line, token);
	Gate* gate = new_gate(word, num_inputs, num_outputs);

	int i;
	for (i = 0; i < num_inputs; i++)
	{
		word = strtok(NULL, token);
		gate->inputs[i] = new_var(word);
	}
	for (i = 0; i < num_outputs; i++)
	{
		word = strtok(NULL, token);
		gate->outputs[i] = new_var(word);
	}
	insert_gate(gate);
}

unsigned int get_hash(char* str)
{
	unsigned int hash = 3181;
	int length = strlen(str);
	int i;
	for (i = 0; i < length; i++)
	{
		hash = 33 * hash + str[i];
	}
	return hash % TABLE_SIZE;
}

void read_file(FILE* fp)
{
	char line[BUFFER];
	int i = 1;

	while (fgets(line, BUFFER, fp) != NULL)
	{
		// printf("Reading line %d : %s\n", i, line);
		parse_line(line);
		i++;
	}
	// printf("\n");
}

void initialize_LL()
{
	gates = (LL*) malloc(1 * sizeof(LL));
	gates->length = 0;
}

void insert_gate(Gate* gate)
{
	Node* n = new_gate_node(gate);
	if (gates->head == NULL)
	{
		gates->head = n;
		gates->tail = n;
	}
	else
	{
		gates->tail->next = n;
		gates->tail = n;
	}
	gates->length++;
}

Var* new_var(char* input_name)
{
	char* name = copy_string(input_name); //clean inputs, should make a seperate function that doesn't allocate extra memory
	// printf("new_var(%s)\n", name);
	unsigned int hash = get_hash(name);
	// printf("\thashes to %x\n", hash);
	Node* curr = vars[hash];
	while (curr != NULL)
	{
		// printf("\t\tcolliding var: %s\n", curr->var->name);
		if (strcmp(curr->var->name, name) == 0) 
		{
			return curr->var;
		}
		curr = curr->next;
	}
	Var* var = (Var*) malloc(1 * sizeof(Var));
	var->name = copy_string(name);
	if (var->name[0] == '0') var->value = 0;
	else if (var->name[0] == '1') var->value = 1;
	else var ->value = -1;

	Node* var_node = new_var_node(var);

	if (vars[hash] == NULL)
	{
		// printf("\tinserting\n");
		vars[hash] = var_node;
	}
	else
	{
		// printf("\tbucket collision\n");
		curr = vars[hash];
		while (curr->next != NULL)
		{
			curr = curr->next;
		}
		// printf("\tinserting after %s\n", curr->var->name);
		curr->next = var_node;
	}

	return var;
}

Gate* new_gate(char* name, int num_inputs, int num_outputs)
{
	Gate* gate = (Gate*) malloc(1 * sizeof(Gate));
	gate->name = copy_string(name);
	gate->inputs = (Var**) malloc(num_inputs * sizeof(Var*));
	gate->num_inputs = num_inputs;
	gate->outputs = (Var**) malloc(num_outputs * sizeof(Var*));
	gate->num_outputs = num_outputs;
	gate->graycode = NULL;
	return gate;
}

void run_circuit()
{
	// printf("Running Circuit...\n");
	Node* curr = gates->head;

	while (curr != NULL)
	{
		Gate* gate = curr->gate;

		// print_gate_abbrev(gate);

		char first_letter = gate->name[0];

		switch(first_letter)
		{
			case 'A': //AND
			{
				gate->outputs[0]->value = gate->inputs[0]->value && gate->inputs[1]->value;
				break;
			}
			case 'D': //DECODER
			{
				char** graycode = gate->graycode;

				int i,j;

				char digit;

				for (i = 0; i < gate->num_outputs; i++)
				{
					gate->outputs[i]->value = 1;
					for (j = 0; j < gate->num_inputs; j++)
					{
						digit = digit_to_char(gate->inputs[j]->value);
						if (digit != graycode[i][j])
						{
							gate->outputs[i]->value = 0;
							break;
						}
					}
				}

				// free_graycode(graycode, gate->num_outputs);
				break;
			}
			case 'M': //MULTIPLEXER
			{
				char* selection = (char*) malloc(gate->num_selectors * sizeof(char) + 1);

				int i;

				for (i = 0; i < gate->num_selectors; i++)
				{
					selection[i] = digit_to_char(gate->selectors[i]->value);
				}
				selection[gate->num_selectors] = '\0';

				char** graycode = gate->graycode;

				for (i = 0; i < gate->num_inputs; i++)
				{
					if (strcmp(selection, graycode[i]) == 0)
					{
						gate->outputs[0]->value = gate->inputs[i]->value;
						break;
					}
				}

				free(selection);
				// free_graycode(graycode, gate->num_inputs);
				break;
			}
			case 'N':
			{
				switch(gate->name[2])
				{
					case 'N': //NAND
					{
					gate->outputs[0]->value = !(gate->inputs[0]->value && gate->inputs[1]->value);
						break;
					}
					case 'R': //NOR
					{
					gate->outputs[0]->value = !(gate->inputs[0]->value || gate->inputs[1]->value);
						break;
					}
					case 'T': //NOT
					{
						gate->outputs[0]->value = !gate->inputs[0]->value;
						break;
					}
				}
				break;
			}
			case 'O': //OR
			{
				gate->outputs[0]->value = gate->inputs[0]->value || gate->inputs[1]->value;
				break;
			}
			case 'X': //XOR
			{
				gate->outputs[0]->value = !gate->inputs[0]->value != !gate->inputs[1]->value;
				break;
			}
			default :
			{
				break;
			}
		}

		// print_gate(gate);
		curr = curr->next;
	}
}

char* copy_string(char* str)
{
	int i;
	for (i=strlen(str)-1; i >=0 ; i--)
	{
		if (!isalnum(str[i])) str[i] = '\0';
		else break;
	}
	char* cpy = (char*) malloc(strlen(str)+1 * sizeof(char));
	strcpy(cpy, str);
	return cpy;
}

char* copy_string_buffered(char* str)
{
	char* cpy = (char*) malloc(BUFFER * sizeof(char));
	strcpy(cpy, str);
	return cpy;
}

char** get_graycode(int target)
{
	char* code[2];
	code[0] = copy_string_buffered("0");
	code[1] = copy_string_buffered("1");

	return generate_graycode(code, 2, target);
}

char** generate_graycode(char** base, int len, int target)
{
	if (len >= target)
	{
		return base;
	}

	int doublelen = 2*len;
	char** code = (char**) malloc(doublelen*sizeof(char*));
	int i, j;

	int digits = lg(len);

	//populate code with base and reverse of base
	for (i = 0; i < len; i++)
	{
		code[i] = base[i];
		code[len+i] = copy_string_buffered(base[len-i-1]);
	}

	//shift digits forward 1
	for (i = 0; i < doublelen; i++)
	{
		for (j = digits + 1; j > 0; j--)
		{
			code[i][j] = code[i][j-1];
		}
	}

	//add leading digit
	for (i = 0; i < len; i++)
	{
		code[i][0] = '0';
		code[i+len][0] = '1';
	}

	return generate_graycode(code, doublelen, target);
}
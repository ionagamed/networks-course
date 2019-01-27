#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define STACK_MAX_SIZE 1024

typedef struct {
	int arr[STACK_MAX_SIZE];
	int next_element;
} custom_stack_t;

custom_stack_t * stack_create() {
	custom_stack_t * stack = malloc(sizeof(custom_stack_t));

	if (!stack) {
		perror("malloc");
	}

	stack->next_element = 0;
	return stack;
}

int stack_size(custom_stack_t * stack) {
	return stack->next_element;
}

void stack_display(custom_stack_t * stack) {
	int i;
	printf("\n");
	for (i = 0; i < stack->next_element; i++) {
		printf("%d ", stack->arr[i]);
	}
	printf("\n");
}

int stack_empty(custom_stack_t * stack) {
	if (stack_size(stack) == 0) {
		return -1;
	} else {
		return 0;
	}
}

int stack_pop(custom_stack_t * stack) {
	if (stack->next_element == 0) {
		return -1;
	} else {
		stack->next_element--;
		return 0;
	}
}

int stack_push(custom_stack_t * stack, int value) {
	if (stack->next_element >= STACK_MAX_SIZE) {
		return -1;
	} else {
		stack->arr[stack->next_element] = value;
		stack->next_element++;
		return 0;
	}
}

int stack_peek(custom_stack_t * stack) {
	if (stack->next_element == 0) {
		return -1;
	} else {
		return stack->arr[stack->next_element - 1];
	}
}

void split_command_arguments(char * command, char * split_command[2]) {
	int i;
	int argument_no = 0;
	int p = 0;
	for (i = 0; command[i] && i < 1024; i++) {
		if (command[i] == ' ') {
			split_command[argument_no][p] = 0;
			argument_no += 1;
			p = 0;
		} else if (command[i] != '\n') {
			split_command[argument_no][p] = command[i];
			p++;
		}
	}
	split_command[argument_no][p] = 0;
}

void child(int fd) {
	char * command = malloc(1024);
	char * split_command[2];
	split_command[0] = malloc(1024);
	split_command[1] = malloc(1024);

	custom_stack_t * stack;

	while (1) {
		read(fd, command, 1024);
		split_command_arguments(command, split_command);

		if (strcmp(split_command[0], "create") == 0) {
			stack = stack_create();
		} else if (strcmp(split_command[0], "push") == 0) {
			stack_push(stack, atoi(split_command[1]));
		} else if (strcmp(split_command[0], "pop") == 0) {
			stack_pop(stack);
		} else if (strcmp(split_command[0], "display") == 0) {
			stack_display(stack);
		} else if (strcmp(split_command[0], "peek") == 0) {
			int value = stack_peek(stack);
			printf("%d\n", value);
		} else if (strcmp(split_command[0], "empty") == 0) {
			int value = stack_empty(stack);
			printf("%d\n", value);
		} else if (strcmp(split_command[0], "size") == 0) {
			int value = stack_size(stack);
			printf("%d\n", value);
		}

		fflush(stdout);
	}
}

void parent(int fd) {
	char * command = malloc(1024);

	while (1) {
		fgets(command, 1024, stdin);
		write(fd, command, 1024);
	}
}

int main() {
	int pipe_fd[2];

	if (pipe(pipe_fd) < 0) {
		perror("pipe");
	}

	int pipe_read = pipe_fd[0];
	int pipe_write = pipe_fd[1];

	int pid = fork();

	if (pid == 0) {
		child(pipe_read);
	} else {
		parent(pipe_write);
	}
}
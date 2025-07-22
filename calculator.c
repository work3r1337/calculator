#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TOKEN_SIZE 10
#define MAX_BUFFER_SIZE 100
#define NUMBER '0'
#define OPERATOR '+'

typedef struct Node {
    char token[MAX_TOKEN_SIZE];
    struct Node *next;
} Node;

typedef struct {
    Node *top;
} Stack;

void tokenization(char str[]);
void infix_to_postfix(char str[]);
double calc_rpn(char str[]);
int getPriority(const char token[]);
int getoken(char **str, char token[]);
char getch(char **str);
void ungetch(char **str);
int isempty(const Stack *stack_p);
void push(Stack *stack_p, const char token[]);
void pop(Stack *stack_p, char token[]);
void clean(Stack *stack_p);

int main() {
    char str[MAX_BUFFER_SIZE];
    int index = 0;
    while ((str[index++] = getchar()) != '\n');
    str[index] = '\0';
    tokenization(str);
    infix_to_postfix(str);
    printf("%lf", calc_rpn(str));
    return 0;
}

void tokenization(char str[]) {
    char buffer[MAX_BUFFER_SIZE];
    char token[MAX_TOKEN_SIZE];
    char *tmp = str;
    int index = 0;
    *strchr(str, '\n') = '\0';
    while (getoken(&tmp, token) != '\0') {
        int i = 0;
        while (token[i] != '\0') buffer[index++] = token[i++];
        buffer[index++] = ' ';
    }
    buffer[--index] = '\0';
    memcpy(str, buffer, MAX_BUFFER_SIZE);
}

void infix_to_postfix(char str[]) {
    int type;
    char buffer[MAX_BUFFER_SIZE];
    char token[MAX_TOKEN_SIZE], tmp_token[MAX_TOKEN_SIZE];
    char *tmp = str;
    int index = 0;
    Stack stack;
    stack.top = NULL;
    while ((type = getoken(&tmp, token)) != '\0') {
        if (type == NUMBER) {
            int i = 0;
            while (token[i] != '\0') buffer[index++] = token[i++];
            buffer[index++] = ' ';
        } else if (strcmp(token, "(") == 0) {
            push(&stack, token);
        } else if (strcmp(token, ")") == 0) {
            pop(&stack, tmp_token);
            while (!isempty(&stack) && strcmp(tmp_token, "(") != 0) {
                int i = 0;
                while (tmp_token[i] != '\0') buffer[index++] = tmp_token[i++];
                buffer[index++] = ' ';
                pop(&stack, tmp_token);
            }
        } else {
            if (!isempty(&stack)) {
                pop(&stack, tmp_token);
                while (!isempty(&stack) &&
                       getPriority(tmp_token) >= getPriority(token)) {
                    int i = 0;
                    while (tmp_token[i] != '\0') buffer[index++] = tmp_token[i];
                    buffer[index++] = ' ';
                    pop(&stack, tmp_token);
                }
                if (getPriority(tmp_token) < getPriority(token))
                    push(&stack, tmp_token);
            }
            push(&stack, token);
        }
    }
    while (!isempty(&stack)) {
        pop(&stack, tmp_token);
        int i = 0;
        while (tmp_token[i] != '\0') buffer[index++] = tmp_token[i++];
        buffer[index++] = ' ';
    }
    buffer[--index] = '\0';
    strcpy(str, buffer);
    clean(&stack);
}

double calc_rpn(char str[]) {
    int type;
    char token[MAX_TOKEN_SIZE];
    char tmp_token1[MAX_TOKEN_SIZE], tmp_token2[MAX_TOKEN_SIZE];
    char tmp[MAX_TOKEN_SIZE];
    Stack stack;
    stack.top = NULL;

    while ((type = getoken(&str, token)) != '\0') {
        if (type == NUMBER)
            push(&stack, token);
        else if (strcmp(token, "+") == 0) {
            pop(&stack, tmp_token1);
            pop(&stack, tmp_token2);
            sprintf(tmp, "%lf", atof(tmp_token1) + atof(tmp_token2));
            push(&stack, tmp);
        } else if (strcmp(token, "*") == 0) {
            pop(&stack, tmp_token1);
            pop(&stack, tmp_token2);
            sprintf(tmp, "%lf", atof(tmp_token1) * atof(tmp_token2));
            push(&stack, tmp);
        } else if (strcmp(token, "-") == 0) {
            pop(&stack, tmp_token1);
            pop(&stack, tmp_token2);
            sprintf(tmp, "%lf", atof(tmp_token2) - atof(tmp_token1));
            push(&stack, tmp);
        } else if (strcmp(token, "/") == 0) {
            pop(&stack, tmp_token1);
            pop(&stack, tmp_token2);
            if (atof(tmp_token1) != 0.0) {
                sprintf(tmp, "%lf", atof(tmp_token2) / atof(tmp_token1));
                push(&stack, tmp);
            } else
                printf("error: zero divisor\n");
        } else
            printf("error: unknown command");
    }
    pop(&stack, tmp_token1);
    clean(&stack);
    return atof(tmp_token1);
}

int getPriority(const char token[]) {
    if (strcmp(token, "+") == 0 || strcmp(token, "-") == 0)
        return 1;
    else if (strcmp(token, "*") == 0 || strcmp(token, "/") == 0)
        return 2;
    else
        return 0;
}

int getoken(char **str, char token[]) {
    int i = 0;
    while ((token[i] = getch(str)) == ' ' || token[i] == '\t');
    token[1] = '\0';
    if (token[i] == '\0') return '\0';
    if (isdigit(token[i])) {
        while (isdigit(token[++i] = getch(str)));
        if (token[i] == '.')
            while (isdigit(token[++i] = getch(str)));
        token[i] = '\0';
        ungetch(str);
        return NUMBER;
    } else {
        while (isalpha(token[++i] = getch(str)));
        token[i] = '\0';
        ungetch(str);
        return OPERATOR;
    };
}

char getch(char **str) { return *(*str)++; }

void ungetch(char **str) { (*str)--; }

int isempty(const Stack *stack_p) { return (stack_p->top == NULL); }

void push(Stack *stack_p, const char token[]) {
    Node *tmp_node = stack_p->top;
    stack_p->top = (Node *)malloc(sizeof(Node));
    stack_p->top->next = tmp_node;
    memcpy(stack_p->top->token, token, MAX_TOKEN_SIZE);
}

void pop(Stack *stack_p, char token[]) {
    memcpy(token, stack_p->top->token, MAX_TOKEN_SIZE);
    Node *tmp = stack_p->top;
    stack_p->top = stack_p->top->next;
    free(tmp);
}

void clean(Stack *stack_p) {
    while (stack_p->top != NULL) {
        Node *tmp_p = stack_p->top;
        stack_p->top = stack_p->top->next;
        free(tmp_p);
    }
}
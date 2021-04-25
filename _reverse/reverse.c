#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <assert.h>

struct Node {
  struct Node* next;
  char data[];
};

char *inputString(FILE*, size_t);
void inputFunc(struct Node** s);
static void reverse(struct Node** s);
void print(struct Node* s);
void readFile(char input[], struct Node** s);
void writeFile(char output[], struct Node** s);
void append(struct Node** s, char new_data[]);

static void reverse(struct Node** head_ref) {

  //Reverse linked list

  struct Node* prev = NULL;
  struct Node* current = *head_ref;
  struct Node* next = NULL;

  while(current != NULL) {
    next = current->next;
    current->next = prev;

    prev = current;
    current = next;

  }

  *head_ref = prev;
}

void print(struct Node* head_ref) {
  struct Node* test = head_ref;
  while(test != NULL) {
    printf("%s\n", test->data);
    test = test->next;
  }
}

//Main with the main logic
int main(int argc, char *argv[]) {

  struct Node* head_ref = NULL;

  if(argc == 1) {
    inputFunc(&head_ref);
    reverse(&head_ref);
    print(head_ref);
  } else if(argc == 2) {
    readFile(argv[1], &head_ref);
    reverse(&head_ref);
    print(head_ref);
  } else if(argc == 3) {
    if(strcmp(argv[1], argv[2]) == 0) {
      fprintf(stderr, "Input and output file must differ\n");
      exit(1);
    }
    readFile(argv[1], &head_ref);
    reverse(&head_ref);
    writeFile(argv[2], &head_ref);
  } else {
    fprintf(stderr, "usage: reverse <input> <output> \n");
    exit(1);
  }

  return 0;
}

void inputFunc(struct Node** head_ref) {
  printf("Enter input string: ");
  char *str;

  str = inputString(stdin, 10);

  //Remove trailing \n (might be obsolete)
  if ((strlen(str) > 0) && (str[strlen (str) - 1] == '\n'))
        str[strlen (str) - 1] = '\0';

  char* token = strtok(str, " \t");

  //Add each word separated by " " to the linked list
  while(token != NULL) {
    append(head_ref, token);
    token = strtok(NULL, " \t");
  }

  free(str);
  free(token);
}

//Allocate more memory to input if needed
char *inputString(FILE* fp, size_t size){
    char *str;
    int ch;
    size_t len = 0;
    str = realloc(NULL, sizeof(*str)*size);//size is start size
    if(!str)return str;
    while(EOF!=(ch=fgetc(fp)) && ch != '\n'){
        str[len++]=ch;
        if(len==size){
            str = realloc(str, sizeof(*str)*(size+=16));
            if(!str)return str;
        }
    }
    str[len++]='\0';

    //return string with enough memory
    return realloc(str, sizeof(*str)*len);
}


void writeFile(char output[], struct Node** head_ref) {
  FILE *fp;
  fp = fopen(output, "w");

  if(fp == NULL) {
    fprintf(stderr, "error: cannot open file '%s'\n", output);
    exit(1);
  }

  struct Node* current = *head_ref;

  while(current != NULL) {
    fprintf(fp, "%s\n", current->data);
    current = current->next;
  }

  fclose(fp);
}



void readFile(char input[], struct Node** head_ref) {
  FILE *fp;
  fp = fopen(input, "r");
  char *buffer = "NULL";

  if(fp == NULL) {
    fprintf(stderr, "error: cannot open file '%s'\n", input);
    exit(1);
  }

  while(strcmp(buffer, "") != 0) {
    buffer = inputString(fp, 10);
    append(head_ref, buffer);
  }

  free(buffer);

  fclose(fp);
}



void append(struct Node** head_ref, char new_data[]) {

  //Give struct all memory new_data would need
  struct Node* new_node = (struct Node*)malloc(sizeof(struct Node) + (strlen(new_data) * sizeof(char *)));

  if(new_node == NULL) {
    fprintf(stderr, "malloc failed\n");
    exit(1);
  }

  struct Node *last = *head_ref;

  strcpy(new_node->data, new_data);

  new_node->next = NULL;

  if(*head_ref == NULL) {
    *head_ref = new_node;
    return;
  }

  while(last->next != NULL) {
    last = last->next;
  }
  last->next = new_node;
  return;
}

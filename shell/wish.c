#include "stdlib.h"
#include "stdio.h"
#include "unistd.h"
#include "string.h"
#include "stddef.h"
#include "assert.h"
#include "sys/stat.h"
#include "fcntl.h"
#include "sys/types.h"
#include "sys/wait.h"

#define PATH "/bin/"
char *search_paths[] = {"/bin/"};
char error_message[30] = "An error has occurred\n";

struct filePaths {
  size_t used;
  size_t size;
  char **path;
};

struct Args {
  struct Args* next;
  int arg_count;
  int is_redirect;
  char arg[];
};

char *inputString(FILE*, size_t);
int processExists(char[], char[]);
void print(struct Args* s);
void appendArgument(struct Args** s, char new_data[], int arg_count, int is_redirect);
void start_process(struct Args* args, int commands, struct filePaths* paths);
int start_process_single_command(struct Args** args, int commands, struct filePaths* paths);
void initArray(struct filePaths* s, size_t c);
void insertArray(struct filePaths* s, char c[]);
void freeArray(struct filePaths* s);
int isMultiple(char s[]);
int isRedirect(char s[]);
void freeList(struct Args* s);

int main(int argc, char *argv[]) {
  int commands = 0;
  struct Args* args = NULL;
  struct filePaths *filepaths = NULL;
  filepaths = malloc(sizeof(struct filePaths));
  if(filepaths == NULL) {
    printf("Memory allocation issues\n");
    return 1;
  }
  initArray(filepaths, 1);
  insertArray(filepaths, "/bin/");
  int x = 1;

  if(argc == 2) { //BATCH CODE
   //int count = 0;
   FILE *fp;
   fp = fopen(argv[1], "r");
   char *buffer = "NULL";
   if(fp == NULL) {
     write(STDERR_FILENO, error_message, strlen(error_message));
     exit(1);
   }

   while(!feof(fp)) {
     buffer = inputString(fp, 10);
     if(strcmp(buffer, "") == 0) {
       break;
     }
     char input[strlen(buffer)];
     char *saveptr;
     strcpy(input, buffer);
     free(buffer);

     /*
     Exit implementation
     */
     if(strcmp(input, "exit") == 0){
       fclose(fp);
       if(filepaths != NULL){
         freeArray(filepaths);
       }
       exit(0);
     }

     args = NULL;

     /*
     Path implementation
     */
     if(strstr(input, "path") != NULL) {
       int arg_counter = 0;
       initArray(filepaths, 1);
       char *args = strtok_r(input, " \t", &saveptr);
       while(args != NULL) {
         args = strtok_r(NULL, " \t", &saveptr);
         if(args != NULL) {
           arg_counter++;
           insertArray(filepaths, args);
         }
       }
       continue;
     }

     /*
     CD Implementation
     */
     if(strstr(input, "cd") != NULL) {
       int arg_counter = 0;
       char *args = strtok_r(input, " \t", &saveptr);
       char *chdir_arg;
       while(args != NULL) {
         chdir_arg = args;
         args = strtok_r(NULL, " \t", &saveptr);
         arg_counter++;
       }
       if(arg_counter > 2 || arg_counter < 2) {
         char *error = "There must be one argument exactly\n";
         write(STDERR_FILENO, error, strlen(error));
         continue;
       }
       int x = chdir(chdir_arg);
       if(x != 0){
         char *error = "Something went wrong\n";
         write(STDERR_FILENO, error, strlen(error));
       }
       continue;
     }

     if(isMultiple(buffer) == 1) {
       int redirect;
       char *tok = strtok_r(input, "&", &saveptr);
       commands = 0;
       while(tok != NULL){
         commands++;
         char *tok1 = strtok_r(tok, " \t", &tok);
         if(isRedirect(tok) == 1) {
           redirect = 1;
         } else {
           redirect = 0;
         }
         int count = 0;
         while(tok1 != NULL) {
           appendArgument(&args, tok1, count, redirect);
           tok1 = strtok_r(NULL, " \t", &tok);
         }
         tok = strtok_r(NULL, "&", &saveptr);
       }
       start_process(args, commands, filepaths);
     } else {
       int redirect;
       if(isRedirect(buffer) == 1) {
         redirect = 1;
       } else {
         redirect = 0;
       }
       char* token = strtok_r(input, " \t", &saveptr);
       int count = 0;
       while(token != NULL) {
         appendArgument(&args, token, count + 1, redirect);
         token = strtok_r(NULL, " \t", &saveptr);
         count++;
       }
       start_process_single_command(&args, count, filepaths);
     }
   }
   fclose(fp);
   if(filepaths != NULL){
     freeArray(filepaths);
   }
   if(buffer != NULL){
     free(buffer);
   }
 } else { //NON-BATCH CODE BEGINS HERE
   char *str;
   while(x == 1) {
     printf("wish> ");
     str = inputString(stdin, 10);
     char input[strlen(str)];
     char *saveptr;
     strcpy(input, str);
     free(str);

     /*
     Exit implementation
     */
     if(strcmp(input, "exit") == 0){
       if(filepaths != NULL){
         freeArray(filepaths);
       }
       exit(0);
     }
     args = NULL;

     /*
     Path implementation
     */
     if(strstr(input, "path") != NULL) {
       int arg_counter = 0;
       initArray(filepaths, 1);
       char *args = strtok_r(input, " \t", &saveptr);
       while(args != NULL) {
         args = strtok_r(NULL, " \t", &saveptr);
         if(args != NULL) {
           arg_counter++;
           insertArray(filepaths, args);
         }
       }
       continue;
     }

     /*
     CD Implementation
     */
     if(strstr(input, "cd") != NULL) {
       int arg_counter = 0;
       char *args = strtok_r(input, " \t", &saveptr);
       char *chdir_arg;
       while(args != NULL) {
         chdir_arg = args;
         args = strtok_r(NULL, " \t", &saveptr);
         arg_counter++;
       }
       if(arg_counter > 2 || arg_counter < 2) {
         char *error = "There must be one argument exactly\n";
         write(STDERR_FILENO, error, strlen(error));
         continue;
       }
       int x = chdir(chdir_arg);
       if(x != 0){
         char *error = "Something went wrong\n";
         write(STDERR_FILENO, error, strlen(error));
       }
       continue;
     }

     if(isMultiple(input) == 1) {
       int redirect;
       char *tok = strtok_r(input, "&", &saveptr);
       commands = 0;
       while(tok != NULL){
         commands++;
         char *tok1 = strtok_r(tok, " \t", &tok);
         int count = 0;
         if(isRedirect(tok) == 1) { //Check if this block of (command + args) should be redirected to file
           redirect = 1;
         } else {
           redirect = 0;
         }
         while(tok1 != NULL) {
           appendArgument(&args, tok1, count, redirect);
           tok1 = strtok_r(NULL, " \t", &tok);
           count++;
         }
         tok = strtok_r(NULL, "&", &saveptr);//, &str);
       }
       start_process(args, commands, filepaths);
     } else {
       int redirect;
       if(isRedirect(input) == 1) {
         redirect = 1;
       } else {
         redirect = 0;
       }
       char* token = strtok(input, " \t");//, &input);
       int count = 0;
       while(token != NULL) {
         appendArgument(&args, token, count + 1, redirect);
         token = strtok(NULL, " \t");//, &input);
         count++;
       }
       start_process_single_command(&args, count, filepaths);
     }
   }
 }
  return 0;
}

int isMultiple(char str[]){
  if(strstr(str, "&") != NULL) {
    return 1;
  } else {
    return 0;
  }

}
int isRedirect(char str[]){
  if(strstr(str, ">") != NULL) {
    return 1;
  } else {
    return 0;
  }
}

//Run processess for multiple commands on single row and their arguments (Linked list, amount of commands)
//Don't try to understand just embrace the spaghetti
void start_process(struct Args* args, int commands_count, struct filePaths* paths){
  struct Args* temp = args;
  struct Args* temp2 = NULL;
  struct Args* new_head = NULL;

  struct filePaths* commands = NULL;
  commands = malloc(sizeof(struct filePaths));
  if(commands == NULL) {
    fprintf(stderr, "Memory allocation issues\n");
    return;
  }
  initArray(commands, 1);

  int arg_count;
  pid_t pid, wpid;
  int status = 0;
  int is_redirect;
  char *path = PATH;
  char *process_check;
  char *argument;
  char *argss;

  //printf("COMMANDS: %d\n", commands_count);

  //Loop through linked argument list and dissect the list as you go
  for (int kids = 0; kids < commands_count; kids++) {

    if(new_head != NULL){
      freeList(new_head);
    }

    new_head = temp;

    while(temp != NULL){
      temp2 = temp;
      insertArray(commands, temp->arg);
      temp = temp->next;
      if(temp == NULL){
        temp2->next = new_head;
        temp2 = temp2->next;
        break;
      }
      if(temp->arg_count == 0) {
        temp2->next = new_head;
        temp2 = temp2->next;
        break;
      }
    }
    if((pid = fork()) == 0) { //Fork processess
      argss = malloc(sizeof(char *)); //Initialize argss each run
      argument = malloc(sizeof(char *)); //Initialize arguments
      do {
        char* tmp = temp2->arg; //Store value
        is_redirect = temp2->is_redirect; //(0 / 1) either output should be redirected to file or not
        strcat(argument, tmp); //store arguments neatly
        strcat(argument, " ");
        temp2 = temp2->next; //travel linked list
      } while(temp2 != new_head);
      char *token = strtok_r(argument, " \t", &argument); //parse arguments
      process_check = malloc((sizeof(path) + sizeof(token))*sizeof(char *)); //Allocate enough memory
      strcat(process_check, path);
      strcat(process_check, token); //Get path and command (ex. /bin/ls)
      arg_count = 0;
      while(token != NULL) { //store arguments into argss
        arg_count++;
        token = strtok_r(argument, " \t", &argument);
        if(token != NULL) {
          argss = realloc(argss, sizeof(token) * sizeof(char *));
          strcat(argss, token);
          strcat(argss, " ");
        }
      }
      free(process_check);
      char* arguments[arg_count + 1]; //Initialize array with the simple counter
      int x;
      //Loop through the paths ('/bin/')
      for(int x1 = 0; x1 < paths->used; x1++){
        char *process_path = malloc(sizeof((*paths->path) + sizeof(*commands->path) * sizeof(char *)));
        strcat(process_path, paths->path[x1]);
        strcat(process_path, commands->path[x1]);
        arguments[0] = strdup(process_path);
        x = access(process_path, X_OK);
        if(x == 0) {
          free(process_path);
          break;
        }
      }
      if(x == 0) { //Success
        //char *asd = strtok_r(argss, " \t", &argss); //Command name
        for(int o = 1; o < arg_count; o++) { //Loopy doopy through the arguments
          char *asd = strtok_r(argss, " \t", &argss);
          //printf("ASD %d: %s\n", o, asd);
          arguments[o] = strdup(asd);
        }
        arguments[arg_count] = NULL; //Last argument is NULL
        if(is_redirect == 1) {
          int xz = 0;
          int bool = 0;
          char *str1;
          for(xz = 0; xz < arg_count; xz++) {
            if(strstr(arguments[xz], ">") != NULL) {
              str1 = strdup(arguments[xz + 1]);
              //printf("%s\n", str1);
              int xc = xz;
              while(xc < arg_count) {
                arguments[xc] = NULL;
                bool = 1;
                xc++;
              }
              if(bool == 1) break;
            }
          }
          int fd = open(str1, O_WRONLY);
          dup2(fd, 1);
          close(fd);
        }
        execv(arguments[0], arguments); //Execute
      } else {
        write(STDERR_FILENO, error_message, strlen(error_message));
      }
      //free(argss);
    } else {
      while((wpid = wait(&status)) > 0);
    }
  }
  freeArray(commands);
  freeList(new_head);
}

//Debugging linked list
void print(struct Args* args) {
  struct Args* test = args;
  while(test != NULL) {
    printf("%s ", test->arg);
    printf("%d\n", test->arg_count);
    test = test->next;
  }
}

//Debugging circular linked list
void printcircular(struct Args* args) {
  struct Args* test = args;
  do {
    printf("%s ", test->arg);
    printf("%d\n", test->arg_count);
    test = test->next;
  } while (test != args);
}

//Allocate more memory to the input if needed
char *inputString(FILE *fp, size_t size) {
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

int start_process_single_command(struct Args** args, int count, struct filePaths* paths) {

  struct Args* temp = *args;
  struct Args* head = *args;
  pid_t pid, wpid;
  int status = 0;
  char *str;
  int is_redirect;

  if(args != NULL) {
    str = temp->arg;
  }
  char *arguments[count + 1];

  int x = 1;
  for(int x1 = 0; x1 < paths->used; x1++) {
    char *process_path = malloc((sizeof(*paths->path) + sizeof(str)) * sizeof(char *));
    //printf("%s\n", paths->path[x1]);
    strcat(process_path, paths->path[x1]);
    strcat(process_path, temp->arg);
    arguments[0] = malloc(sizeof(process_path) * sizeof(char *));
    strcpy(arguments[0], process_path);
    //arguments[0] = strdup(process_path);
    //printf("ARGUMENT: %s\n", arguments[0]);
    x = access(arguments[0], X_OK);
    //printf("x: %d\n", x);
    free(process_path);
    if(x == 0){
      break;
    }
  }

  int i = 0;

  temp = temp->next;
  while(temp != NULL) {
    i++;
    //arguments[i] = strdup(temp->arg);
    char *ptr = temp->arg;
    arguments[i] = malloc(sizeof(ptr) * sizeof(char *));
    strcpy(arguments[i], ptr);
    is_redirect = temp->is_redirect;
    temp = temp->next;
  }
  arguments[i + 1] = NULL; //Last argument == NULL
  int xz = 0;
  int bool = 0;
  char *str1;
  for(xz = 0; xz < i; xz++) {
    if(strstr(arguments[xz], ">") != NULL) {
      str1 = strdup(arguments[xz + 1]);
      int xc = xz;
      while(xc < i) {
        arguments[xc] = NULL;
        bool = 1;
        xc++;
      }
      if(bool == 1){
        break;
      }
      }
    }
  if(x == 0) { //Success
    if((pid = fork()) == 0) {
      if(is_redirect == 1){
        int fd = open(str1, O_WRONLY);
        dup2(fd, 1);
        close(fd);
        free(str1);
      }
       execv(arguments[0], arguments); //Execute
    } else {
      while((wpid = wait(&status)) > 0);

      for(int teemp = 0; teemp <= i + 1; teemp++){
        //printf("%d\n", teemp);
        free(arguments[teemp]);
      }
      //free(arguments);
      freeList(head);
      return 0;
    }
  } else {
    write(STDERR_FILENO, error_message, strlen(error_message));
  }
  return 0;

}

void freeList(struct Args* head) {
  //printcircular(head);
  struct Args* tmp = head, *next;
  do{
    //printf("ARGUMENT: %s\n", tmp->arg);
    next = tmp->next;
    if(next == NULL || tmp == NULL){
    //  printf("LAST ARGUMENT: %s\n", tmp->arg);
      free(tmp);
      break;
    } else {
      free(tmp);
      tmp = next;
    }

  }while(tmp != head);
}

/*Since we cannot know how many arguments and how long those will be use linked list to store commands + arguments*/
void appendArgument(struct Args** args, char argument[], int arg_count, int is_redirect){
  //gibe memory
  struct Args* new_arg = malloc(sizeof(struct Args));

  //printf("%s\n", argument);

  if(new_arg == NULL) {
    write(STDERR_FILENO, error_message, strlen(error_message));
    exit(1);
  }
  struct Args *last = *args;
  strcpy(new_arg->arg, argument);
  new_arg->arg_count = arg_count;
  if(is_redirect == 1){
    new_arg->is_redirect = 1;
  } else {
    new_arg->is_redirect = 0;
  }

  //printf("%d\n", new_arg->is_redirect);

  new_arg->next = NULL;
  if(*args == NULL) {
    *args = new_arg;
    return;
  }

  while(last->next != NULL) {
    last = last->next;
  }
  last->next = new_arg;
  return;
}

/*Dynamic array helper functions*/
void initArray(struct filePaths *a, size_t initialSize) {
  a->path = malloc(initialSize * sizeof(char *) + 1);
  if(a->path == NULL) {
    printf("ERROR: Memory allocation failure!\n");
    exit(1);
  }
  a->used = 0;
  a->size = initialSize;
}

void insertArray(struct filePaths *a, char *search_path) {
  if(a->used == a->size) {
    void *pointer;

    a->size *= 2;
    pointer = realloc(a->path, a->size * sizeof(char *));
    if(a->path == NULL){
      freeArray(a);
      printf("ERROR: Memory allocation failure!\n");
      exit(1);
    }
    a->path = pointer;
  }
  if(search_path != NULL) {
    size_t length;
    length = strlen(search_path);
    a->path[a->used] = malloc(1 + length);
    if(a->path[a->used] != NULL) {
      strcpy(a->path[a->used++], search_path);
    }
  } else {
    a->path[a->used++] = NULL;
  }
}

void freeArray(struct filePaths *a) {
  size_t i;
  for (i = 0; i < a->used; i++) {
    free(a->path[i]);
  }
  free(a->path);
  free(a);
}

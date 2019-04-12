#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#define BUF_SIZE 512
#define DELIM " \t\r\n\a"
#define DELIM1 ";&&"
#define DELIM2 ";"
#define DELIM3 "&&"

// Read input from stdin
char *readLine(void){

  char *buffer = malloc(sizeof(char) * BUF_SIZE);

  if (!buffer) {
    fprintf(stderr, "myshell: allocation error\n");
    exit(EXIT_FAILURE);
  }

  fgets(buffer , BUF_SIZE, stdin);

  if ( strcmp(buffer , "quit\n") == 0){
    exit(EXIT_SUCCESS);
  }

  return buffer;

}
// Split the arguments of the command
char **splitLine(char *line){

  int i = 0 ;
  char **tokens = malloc(BUF_SIZE*sizeof(char*)); // Store the arguments
  char *token;

   // Check for allocation error
  if (!tokens) {
    fprintf(stderr, "myshell: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line , DELIM);

  while(token != NULL){
    tokens[i] = token;
    i++;
    token = strtok(NULL,DELIM);
  }
  tokens[i] = NULL;



  return tokens;

}

int execute(char **args, int fl){

  // Empty command
  if(args[0] == NULL){
    return 1;
  }
  pid_t pid;
  int status,fd[2];
  int k = 0 ;

  char line[BUF_SIZE];

  if ( pipe(fd) < 0){
    perror("pipe error");
    exit(EXIT_FAILURE);
  }


  pid = fork();

  if (pid == 0) {
  // Child process
    if (execvp(args[0], args) == -1) {
      perror("myshelgdfl");

      if (fl == 0){
        close(fd[0]);

        write( fd[1] , "message through pipe\n", 21);

        close(fd[1]);

      }
    }
    exit(EXIT_SUCCESS);

  } else if (pid < 0) {
    // Error forking
    perror("myshell");
  } else {
    // Parent process
    wait(&status);

    if ( fl == 0){
      close(fd[1]);

      k = read(fd[0] , line , BUF_SIZE);

      close(fd[0]);

    }

    if ( k != 0 && k != -1){
      return 0 ;

    }


  }

return 1;

}

// Return the number of consecutive commands
int nConsecutivecmds(char *line){

  char *token;
  int i = 0;
  char *copyline = malloc(sizeof(char) * (strlen(line) + 1));
  memcpy(copyline , line , strlen(line) + 1); // Copy the contents of line


  token = strtok(copyline, DELIM1);

  if(strcmp(token,line) == 0){


    free(copyline);
    return 0 ;

  }else{
    while(token != NULL){
      i++;
      token = strtok(NULL,DELIM1);

    }

    free(copyline);
    return i;

  }


}

int consecutiveCmds(char *line , int ncmds){
  int stat;
  int fl;
  char **args;
  char *token1;
  char *copytoken;
  char *saveptr;
  char delims[2] = {';','&'};
  char **linedelims = malloc(sizeof(char*) * BUF_SIZE);

  if (!linedelims) {
    fprintf(stderr, "myshell: allocation error\n");
    exit(EXIT_FAILURE);
  }

  int z;
  int c1 = 0;
  int c2 = 0;
  int c3 = 0;
  for(z = 0 ; z < strlen(line) ; z++){

    if( line[z] == delims[0]){
      c1 = c1 + 1;
    }

    if (line[z] == delims[1] && line[z+1] == delims[1]){
      c2 = c2 + 1;

    }

    if (line[z] == delims[1]){
      c3 = c3 + 1;
    }

  }
  // Cancel commands with invalid delimiters , ex. ls ;;; pwd or ls &&&& pwd
  if ( c1 >= 2 || c2 >= 2 || c3 == 1){
    printf("Ιnvalid command\n");
    return 1;
  }
  // Cancel commands which start with ; or &&/&
  if (line[0] == delims[0] || line[0] == delims[1]){
    printf("Ιnvalid command\n");
    return 1;
  }


  int j=0;
  int i;

  for(i = 0 ; i < strlen(line) ; i++){
    if ( line[i] == delims[0]){
      linedelims[j] = DELIM2;
      j++;
    }
    if( line[i] == delims[1] && line[i+1] == delims[1]){
      linedelims[j] = DELIM3;
      j++;
    }
  }

  int k = 0 ;

  if (linedelims[k] == DELIM2){
    fl = 1;
  }else{
    fl = 0;
  }


  token1 = strtok_r(line,linedelims[k],&saveptr);
  args = splitLine(token1);
  stat = execute(args,fl);
  printf("\n");
  free(args);
  if (stat == 0){
    return 1;
  }
  k++;

  while(k < (ncmds - 1)){

    if (linedelims[k] == DELIM2){
      fl = 1;
    }else{
      fl = 0;
    }

    token1 = strtok_r(NULL,linedelims[k],&saveptr);
    args = splitLine(token1);
    stat = execute(args,fl);
    printf("\n");
    free(args);

    if (stat == 0){
      return 1;
    }
    k++;
  }

  k--;
  token1 = strtok_r(NULL,linedelims[k],&saveptr);
  args = splitLine(token1);
  stat = execute(args, fl);
  printf("\n");
  free(args);
  if (stat == 0 ){
    return 1;
  }

  return stat;

}

int redirectCmds(char **args){

  pid_t pid;
  int stat;

  pid = fork();

  if(pid == 0){
    //Child process
    int fd0,fd1,in,out,i;
    char input[BUF_SIZE],output[BUF_SIZE];

    for ( i = 0 ; args[i] != NULL ; i++){

      if(strcmp(args[i],"<") == 0){
        args[i] = NULL;
        strcpy(input,args[i+1]);
        in = 1;
      }

      if(strcmp(args[i],">") == 0){
        args[i] = NULL;
        strcpy(output,args[i+1]);
        out = 1;
      }
    }

    if(in){
      fd0 = open(input, O_RDONLY | O_CREAT);
      if(fd0 < 0){
        perror("myshell");
        exit(EXIT_FAILURE);

      }
      dup2(fd0,STDIN_FILENO);
      close(fd0);

    }

    if(out){
      fd1 = creat(output, S_IRUSR | S_IWUSR);
      if(fd1 < 0){
        perror("myshell");
        exit(EXIT_FAILURE);
      }

      dup2(fd1,STDOUT_FILENO);
      close(fd1);
    }

    if(execvp(*args,args) == -1){
      perror("myshell");
    }
    exit(EXIT_SUCCESS);

  //Error forking
  }else if( pid < 0){
    perror("myshell");

  //Parent process
  }else{
    wait(&stat);
  }

  return 1;

}

//Main function
int main(int argc, char const *argv[]) {

  char *line;
  int status;
  FILE *fp;
  char **args;

  int ncmds; // Number of consecutive commands
  int fl  = 1; // Flag for communatication between processes

  if(argc == 1){

    do{

      printf("kalantzis_8818>");
      line = readLine();


      //Redirection command
      if(strchr(line,'<') != NULL | strchr(line,'>') != NULL){
        args = splitLine(line);
        status = redirectCmds(args);
        continue;

      }
      //Check for commands with special characters { ; , &&}
      ncmds = nConsecutivecmds(line);

      // Command without special character
      if (ncmds == 0){
        args = splitLine(line);
        status = execute(args, fl);
        free(line);
        free(args);
      // Command with special character
      }else{

        status = consecutiveCmds(line,ncmds);
        free(line);

      }


    } while(status);


  }else{

    fp = fopen(argv[1], "r");
    // File error
    if (fp == NULL){
      perror("Error opening file");
      return 0;
    }

    line = malloc(sizeof(char) * BUF_SIZE);
    // Allocation error
    if (!line) {
      fprintf(stderr, "myshell: allocation error\n");
      exit(EXIT_FAILURE);
    }

    fgets(line,BUF_SIZE,fp);

    do{

      // Skip newlines
      if(strcmp(line,"\n") == 0) continue;

      //Exit signal
      if ( strcmp(line, "quit\n") == 0){
        exit(EXIT_SUCCESS);
      }

      //Redirection command
      if(strchr(line,'<') != NULL | strchr(line,'>') != NULL){
        args = splitLine(line);
        status = redirectCmds(args);
        continue;

      }
      //Check for commands with special characters { ; , &&}
      ncmds = nConsecutivecmds(line);

      // Command without special character
      if (ncmds == 0){
        args = splitLine(line);
        status = execute(args, fl );
        free(line);
        free(args);
      // Command with special character
      }else{

        status = consecutiveCmds(line,ncmds);
        free(line);

      }

      line = malloc(sizeof(char) * BUF_SIZE);
      // Allocation error
      if (!line) {
        fprintf(stderr, "myshell: allocation error\n");
        exit(EXIT_FAILURE);
      }


    }while(fgets(line,BUF_SIZE,fp) != NULL);

    fclose(fp);
    free(line);

  }

  return 0;
}

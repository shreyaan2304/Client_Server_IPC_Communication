#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
 
#define REQUEST_TYPE 1
#define RESPONSE_TYPE 2
#define TERMINATION_TYPE 3
 
struct msg_buffer {
   long msg_type;
   char msg_text[100];
   int cid;
} message;
 
 
void handle_client_request(int pipe_write_end, struct msg_buffer *request, int msg_id, int cid) {
   char response_text[100];
 
   if (strcmp(request->msg_text, "hi") == 0) {
      printf("Server child %d handling the ping request for client %d\n", getpid(), cid);
      strcpy(response_text, "hello");
   } else if (strncmp(request->msg_text, "search", 6) == 0) {
      printf("Server child %d handling the file serarch request for client %d\n", getpid(), cid);
      char* filename = request->msg_text + 7;
      struct stat buffer;
      int file_exists = stat(filename, &buffer) == 0;
      
      if (file_exists) {
         sprintf(response_text, "File '%s' found.", filename);
      } else {
         sprintf(response_text, "File '%s' not found.", filename);
      }
   } else if (strncmp(request->msg_text, "count", 5) == 0) {
      printf("Server child %d handling the file word count request for client %d\n", getpid(), cid);
      char* filename = request->msg_text + 6;
      
      int fd[2];
      pipe(fd);
      
      if (fork() == 0) {
         close(fd[0]);
         dup2(fd[1], STDOUT_FILENO);
         
         if(execlp("wc", "wc", "-w", filename, NULL) == -1) {
         	perror("execlp");
         	exit(1);
         };
         
         exit(0);
      } else {
         close(fd[1]);
         
         char buffer[100];
         read(fd[0], buffer, sizeof(buffer));
         
         sprintf(response_text, "Number of words are :  %s", buffer);
     }
   }
     
   message.msg_type = RESPONSE_TYPE;
   strcpy(message.msg_text, response_text);
   
   if(msgsnd(msg_id, &message, sizeof(message), 0) == -1) {
   		perror("msgsnd");
   		exit(1);
   };
}
 
 
int main() {
   key_t key;
   int msg_id;
 
   key = ftok("progfile", 65);
 
   msg_id = msgget(key, 0666 | IPC_CREAT);
   
   while(1) {
      if(msgrcv(msg_id, &message, sizeof(message), REQUEST_TYPE, 0) == -1) {
      		perror("msgrcv");
      		exit(1);
      };
      
      if (strcmp(message.msg_text, "terminate") == 0) {
         if(msgctl(msg_id, IPC_RMID, NULL) == -1) {
         	perror("msgctl");
         	exit(1);
         };
         return 0;
      }
 
      int pipefd[2];
      if (pipe(pipefd) == -1) {
         perror("pipe");
         exit(EXIT_FAILURE);
      }
 
      pid_t pid = fork();
      
      if(pid == -1) {
      	perror("fork");
      	return 1;
      }
      
      if (pid == 0) {
         close(pipefd[0]);  
         handle_client_request(pipefd[1], &message, msg_id, message.cid);
         
         close(pipefd[1]); 
 
         exit(0);
      } else if (pid > 0) {
         close(pipefd[1]);  
         
         close(pipefd[0]);
 
         wait(NULL);
      }
   }
   
   return 0;
}

#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <string.h>
 
#define REQUEST_TYPE 1
#define TERMINATION_TYPE 3
 
struct msg_buffer {
   long msg_type;
   char msg_text[100];
} message;
 
int main() {
   key_t key;
   int msg_id;
   
   key = ftok("progfile", 65);
 
   msg_id = msgget(key, 0666 | IPC_CREAT);
   
   while(1) {
      char option;
      printf("Do you want the server to terminate? Press Y for Yes and N for No.\n");
      scanf(" %c", &option);
      
      if(option == 'Y' || option == 'y') {
         message.msg_type = REQUEST_TYPE;
         strcpy(message.msg_text, "terminate");
         if(msgsnd(msg_id, &message, sizeof(message), 0) == -1) {
         	perror("msgsnd");
         	exit(1);
         };
         
         return 0;
      }
   }
   
   return 0;
}

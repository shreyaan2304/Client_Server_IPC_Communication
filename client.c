#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
 
#define REQUEST_TYPE 1
#define RESPONSE_TYPE 2
 
struct msg_buffer {
    long msg_type;
    char msg_text[100];
    int cid;
} message;
 
int main() {
    key_t key;
    int msg_id;
    int client_id;
 
    key = ftok("progfile", 65);
 
    if (key == -1) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }
 
    msg_id = msgget(key, 0666 | IPC_CREAT);
 
    if (msg_id == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }
 
    printf("Enter Client-ID:\n");
    scanf("%d", &client_id);
    if(client_id < 0) {
    	printf("Client ID must be a positive integer\n");
    	return 1;
    }
	
    message.cid = client_id;
 
    while (1) {
        int option;
        printf("1. Enter 1 to contact the Ping Server\n");
        printf("2. Enter 2 to contact the File Search Server\n");
        printf("3. Enter 3 to contact the File Word Count Server\n");
        printf("4. Enter 4 if this Client wishes to exit\n");
        if (scanf("%d", &option) != 1) {
            printf("Invalid input for option.\n");
            exit(EXIT_FAILURE);
        }
		
		
        switch (option) {
            case 1:
                message.msg_type = REQUEST_TYPE;
                strcpy(message.msg_text, "hi");
                if (msgsnd(msg_id, &message, sizeof(message), 0) == -1) {
                    perror("msgsnd");
                    exit(EXIT_FAILURE);
                }
                break;
            case 2:
                char filename[100];
                printf("Enter filename:\n");
                if (scanf("%99s", filename) != 1) {
                    printf("Invalid input for filename.\n");
                    exit(EXIT_FAILURE);
                }
 
                message.msg_type = REQUEST_TYPE;
                sprintf(message.msg_text, "search %s", filename);
                if (msgsnd(msg_id, &message, sizeof(message), 0) == -1) {
                    perror("msgsnd");
                    exit(EXIT_FAILURE);
                }
                break;
            case 3:
                printf("Enter filename:\n");
                if (scanf("%99s", filename) != 1) {
                    printf("Invalid input for filename.\n");
                    exit(EXIT_FAILURE);
                }
 
                message.msg_type = REQUEST_TYPE;
                sprintf(message.msg_text, "count %s", filename);
                if (msgsnd(msg_id, &message, sizeof(message), 0) == -1) {
                    perror("msgsnd");
                    exit(EXIT_FAILURE);
                }
                break;
            case 4:
               return 0;
            default:
                printf("Invalid option\n");
        }
 
        if (msgrcv(msg_id, &message, sizeof(message), RESPONSE_TYPE, 0) == -1) {
            perror("msgrcv");
            exit(EXIT_FAILURE);
        }
 
        printf("%s\n", message.msg_text);
    }
    return 0;
}

	#include <string.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netinet/ip.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <dirent.h>

	#define MAXLINE 4096


	int main(int argc, char *argv[]) {
		if (argc != 3) { // check if arguments are provided
			printf("Please provide the server host name and server port number as two arguments\n");
			exit(1);
		}

		int sock, put_sock;
		struct sockaddr_in server_addr;
		int port = atoi(argv[2]);

		if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) { // tcp protocol
			perror("Error: socket creation failed\n");
			exit(1);
		}

		server_addr.sin_addr.s_addr = inet_addr(argv[1]);
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(port); // convert into Big Endian format
		// inet_aton("127.0.0.1", &server_addr.sin_addr);

		if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
			perror("Error: connect failed\n");
			exit(1);
		}

		printf("Please enter username and password authentications\n");

		// start the while loop
		while (1) {
			printf("ftp> ");
			char *cmd, *cmd_arg, *sendCopy, *sendline, *reply;
			cmd = malloc(MAXLINE);
			cmd_arg = malloc(MAXLINE);
			sendCopy = malloc(MAXLINE);
			sendline = malloc(MAXLINE);
			reply = malloc(MAXLINE);


			fgets(sendline, MAXLINE, stdin);
			strcpy(sendCopy, sendline); // copy the command so that the original line is not altered
			cmd = strtok(sendCopy, " \n");
			cmd_arg = strtok(NULL, " \n");


			if (strcmp("QUIT", cmd) == 0) { // if the command is QUIT
				write(sock, sendline, strlen(sendline) + 1); 
				read(sock, reply, 25);
				if (strcmp(reply,"CLOSE") == 0){
					close(sock);
					_exit(0);
				}

			}
			else if (strcmp("USER", cmd) == 0) { // authenticate the username
				write(sock, sendline, strlen(sendline) + 1);
				read(sock, reply, 25);
				if (strcmp(reply, "OK") == 0) {
					printf("User Authenticated\n");
				}
				else {
					printf("User does not exist. Please try again\n");
				}
			}
			else if (strcmp("PASS", cmd) == 0) { // password authentication
				write(sock, sendline, strlen(sendline) + 1);
				read(sock, reply, 25);
				if (strcmp(reply, "OK") == 0) {
					printf("Password Accepted\n");
				}
				else if (strcmp(reply, "NOUSER") == 0){
					printf("Please enter a username first\n");
				}
				else {
					printf("Password Failed\n");
				}
			}
			else if (strcmp("PWD", cmd) == 0) {
				write(sock, sendline, strlen(sendline) + 1);
				read(sock, reply, MAXLINE);
				if (strcmp(reply, "FAILED") == 0) {
					printf("Cannot print server working directory\n");
				}
				else if (strcmp(reply, "NOPASS") == 0){
					printf("Please enter your password first\n");
				}
				else {
					printf("%s\n", reply); // print server pwd
				}
			}


			else if (strcmp("PUT", cmd) == 0) {
				if (access(cmd_arg, F_OK) == 0) {
					char reply[] = "EXISTS";
					write(sock, reply, strlen(reply) + 1);

					//Start new socket connection for client
					struct sockaddr_in put_addr;
					int put_sock = socket(AF_INET, SOCK_STREAM, 0);
			    if (put_sock == -1) {
					printf("Could not create put socket");
				}

					put_addr.sin_addr.s_addr = inet_addr(argv[1]);
				put_addr.sin_family = AF_INET;
				    put_addr.sin_port = htons(5000);

				//Connect to remote server
					if (connect(put_sock, (struct sockaddr *)&put_addr, sizeof(put_addr)) < 0) {
						perror("connect to put sock failed. Error");
						return 1;
				}

					//End new socket connection for client

					int data = 1;
					char buffer[MAXLINE];
					FILE *fp;

					while (data) {
						fp = fopen(cmd_arg, "r");
						fscanf(fp, "%s", buffer);
			    write(put_sock, buffer, strlen(buffer) + 1); 
						fclose(fp);
						data = 0;
					}
					close(put_sock);
				}
				else {
					printf("%s: file does not exist in client\n", cmd_arg);
				}
			}
			else if (strcmp("!PWD", cmd) == 0) {
				system("pwd");
			}
			
			else if (strcmp("LS", cmd) == 0) {
				write(sock, sendline, strlen(sendline) + 1); 
                                read(sock, reply, MAXLINE);
				if(strcmp(reply,"NOPASS") == 0){
					printf("Please enter your password first\n");
				}
				else{
					printf("%s\n", reply);
				}
			}
			else if (strcmp("!LS", cmd) == 0) {
                        	system("ls");

		}
		else if (strcmp("CD", cmd) == 0) {
			write(sock, sendline, strlen(sendline) + 1);
			read(sock, reply, MAXLINE);
			if (strcmp(reply, "FAILED") == 0) {
				printf("Cannot get current working directory for CD\n");
			}
			else if (strcmp(reply, "NOPASS") == 0) { 
                printf("Please enter your password first\n");
            }
			else {
				char *temp = strchr(reply, '/');
				if (strcmp(cmd_arg, "..") == 0) { // go up one directory
					*temp = '\0';
					chdir(reply);
				}
				else {
					strcat(reply, "/");
					strcat(reply, cmd_arg);
					chdir(reply);
				}
			}
		}
		else if (strcmp("!CD", cmd) == 0) {
			char *cwd = getenv("PWD");
			char *temp = strchr(cwd, '/');
			if (strcmp(cmd_arg, "..") == 0) {
				*temp = '\0';
				cwd = strcat("cd ", cwd); // form the full command
				system(cwd);
			}
			else {
				strcat(cwd, "/");
				strcat(cwd, cmd_arg);
				cwd = strcat("cd ", cwd); // form the full command
				system(cwd);
			}
		}
		else if (strcmp("GET", cmd) == 0) {
			write(sock, sendline, strlen(sendline) + 1);
			read(sock, reply, MAXLINE);
			if (strcmp(reply, "EXISTS") == 0) {
		         if (strcmp(reply, "FAILED") == 0) {
                        printf("Cannot get file on the server\n");
                }
                else if (strcmp(reply, "NOPASS") == 0){ 
                        printf("Please enter your password first\n");
                }
                else {
					//Start new socket connection for client
					struct sockaddr_in get_addr;
					int get_sock = socket(AF_INET , SOCK_STREAM , 0);
        		    if (get_sock == -1) {
                			printf("Could not create get socket");
            		}
            		// printf("GET Socket created\n");

					get_addr.sin_addr.s_addr = inet_addr(argv[1]);
			        get_addr.sin_family = AF_INET;
 			    	get_addr.sin_port = htons(6000);

           			//Connect to remote server
       			 	if (connect(get_sock, (struct sockaddr *)&get_addr, sizeof(get_addr)) < 0) {
  			        	perror("connect to get sock failed. Error");
   			        	return 1;
        			}

   			        // printf("Connected to GET Sock\n");
					//End new socket connection for client

					int data = 1;
					char buffer[MAXLINE];
					FILE *fp;

						fp = fopen(cmd_arg, "w");
						read(get_sock, buffer, MAXLINE);
						fprintf(fp, buffer, strlen(buffer) + 1);
						fclose(fp);
					close(get_sock);
				}
			}
			else if (strcmp(reply, "DOES NOT EXIST") == 0) {
				printf("%s: no such file on server\n", cmd_arg);
			}
		}
		else {
			printf("An invalid ftp command.\n");
		}
	}
}

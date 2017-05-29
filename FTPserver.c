#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdlib.h>

#define MAXLINE 4096

int main()
{
	struct sockaddr_in myaddr;
	struct sockaddr_in cli;
	int new_client,master_sock, conntfd;
	fd_set master_fds, readfds;
	int len = sizeof(cli);
	int max_clients = 10;
	char str[100];
	time_t tick;
	char *request, *cmd, *cmd_arg;
	request = malloc(MAXLINE);
	cmd = malloc(MAXLINE);
	cmd_arg = malloc(MAXLINE);
	char username[] = "NickPeter", password[] = "WhiteChen";
	FILE *fdread;
	master_sock=socket(AF_INET,SOCK_STREAM,0);



	bzero(&myaddr, sizeof(myaddr));
	myaddr.sin_family=AF_INET;
	myaddr.sin_port=htons(5000);

	bind(master_sock,(struct sockaddr*)&myaddr, sizeof(myaddr));

	listen(master_sock,5);

	int clientlist[max_clients][3];


	for (int j=0;j<max_clients;j++){
		for (int z=0;z<3;z++){	
			clientlist[j][z] = -1;
		}
	}

	FD_ZERO(&master_fds);   // FD_ZERO(&master);

	FD_SET(master_sock, &master_fds);

	int max_fd = 0;


	max_fd = master_sock;

	while(1){
		readfds = master_fds;
		select(max_fd + 1, &readfds,NULL,NULL, NULL);

		for(int i=0;i<max_fd+1;i++) //max_fd +1
		{
			if (i == master_sock)
			{
				if (FD_ISSET(master_sock,&readfds)){
					new_client = accept(master_sock,(struct sockaddr *) &cli, &len);
					if (new_client < 0){
						perror("accept");
					}
					FD_SET(new_client, &master_fds);
					if (new_client > max_fd){
						max_fd = new_client;
					}
					clientlist[new_client][0] = new_client; //new_client
					clientlist[new_client][1] = 0;	//didnt exist
					clientlist[new_client][2] = 0;
				}
			} else {
				
				int sockfd = clientlist[i][0];
				if (FD_ISSET(sockfd,&readfds)){ //sockfd
					fdread = fdopen(sockfd, "r");
					fgets(request, MAXLINE, fdread);
					cmd = strtok(request, " \n");
					cmd_arg = strtok(NULL, " \n");
					if (strcmp("QUIT", cmd) == 0){ 
						char reply[] = "CLOSE";
                                                write(sockfd, reply, strlen(reply) + 1);
						close(sockfd);
//                                                FD_CLR(sockfd,&master_sock);
                                                for (int q=0;q<3;q++){
                                                        clientlist[sockfd][q] = -1; 
                                                }
						
                                        }
					if (strcmp("USER", cmd) == 0) {
						if (strcmp(cmd_arg, username) == 0) {
							char reply[] = "OK";
							write(sockfd, reply, strlen(reply) + 1);
							clientlist[sockfd][1] = 1;
						}
						else {
							char reply[] = "FAILED";
							write(sockfd, reply, strlen(reply) + 1);
						}
					}
					else if (strcmp("PASS", cmd) == 0) {
						if (clientlist[sockfd][1] == 1){
							if (strcmp(cmd_arg, password) == 0) {
								char reply[] = "OK";
								write(sockfd, reply, strlen(reply) + 1);
								clientlist[sockfd][2] = 1;
							}
							else {
								char reply[] = "FAILED";
								write(sockfd, reply, strlen(reply) + 1);
							}
						}
						else{
							char reply[] = "NOUSER";
							write(sockfd, reply, strlen(reply) + 1);
						}
					}
					else if (strcmp("PWD", cmd) == 0) {
						if (clientlist[sockfd][2] == 1){
							char *cwd = getenv("PWD");
							if (cwd != NULL) {
								write(sockfd, cwd, strlen(cwd) + 1);
							}
							else {
								char reply[] = "FAILED";
								write(sockfd, reply, strlen(reply) + 1);
							}
						}
						else {
                            char reply[] = "NOPASS";
                            write(sockfd, reply, strlen(reply) + 1); 
                        }
					}
					 else if (strcmp("PUT", cmd) == 0) {
						char reply[MAXLINE];
						read(sockfd, reply, strlen(reply) + 1);
						printf("%s\n", reply);

						if (strcmp(reply, "EXISTS") == 0) {
							// open a new connection
                            if (clientlist[sockfd][2] == 1) {
								int put_sock, data = 1;
								char buffer[MAXLINE];
								FILE *fp;

								//creating put sock here
								struct sockaddr_in putaddr;
								struct sockaddr_in putcliaddr;
								put_sock=socket(AF_INET,SOCK_STREAM,0);


        						putaddr.sin_family=AF_INET;
        						putaddr.sin_port=htons(5000);

        						bind(put_sock,(struct sockaddr*)&putaddr, sizeof(putaddr));


       							listen(put_sock, 5);

								int putclilen = sizeof(putcliaddr);

								//done creating get sock
								while (data) {
									int putclient_fd = accept(put_sock, (struct sockaddr *)&putcliaddr, &putclilen);
									fp = fopen(cmd_arg, "r");
									fscanf(fp, "%s", buffer);
	                                write(putclient_fd, buffer, strlen(buffer) + 1); 
									fclose(fp);
									data = 0;
								}
							}
                           	else {
                                char reply[] = "NOPASS";
                                write(sockfd, reply, strlen(reply) + 1); 
                        	}
	                    }
                    }
					
					else if (strcmp("LS", cmd) == 0) {
						if (clientlist[sockfd][2] == 1){
							FILE *in;
							extern FILE *popen();
							char buff[512];
							char files[MAXLINE] = "";
	
							if (!(in = popen("ls", "r"))) {
								exit(1);
							}
							while (fgets(buff, sizeof(buff), in) != NULL) {
								buff[strcspn(buff, "\n")] = '\t';
								strcat(files, buff);
							}
							write(sockfd, files, strlen(files) + 1);
							pclose(in);
						}
						else{
							char reply[] = "NOPASS";
                                			write(sockfd, reply, strlen(reply) + 1); 
                                		}

					}
					else if (strcmp("GET", cmd) == 0) {
						if (access(cmd_arg, F_OK) == 0) {
							char reply[] = "EXISTS";	
							write(sockfd, reply, strlen(reply) + 1);
							// open a new connection
	 	                           if (clientlist[sockfd][2] == 1) {
								int get_sock, data = 1;
								char buffer[MAXLINE];
								FILE *fp;

								//creating put sock here
								struct sockaddr_in getaddr;
								struct sockaddr_in getcliaddr;
								get_sock=socket(AF_INET,SOCK_STREAM,0);


        						getaddr.sin_family=AF_INET;
        						getaddr.sin_port=htons(6000);

        						bind(get_sock,(struct sockaddr*)&getaddr, sizeof(getaddr));


       							listen(get_sock, 5);

								int getclilen = sizeof(getcliaddr);

								//done creating get sock
									int getclient_fd = accept(get_sock, (struct sockaddr *)&getcliaddr, &getclilen);
									fp = fopen(cmd_arg, "r");
									fscanf(fp, "%s", buffer);
	                                write(getclient_fd, buffer, strlen(buffer) + 1); 
									fclose(fp);
							}
                           	else {
                                char reply[] = "NOPASS";
                                write(sockfd, reply, strlen(reply) + 1); 
                        	}
						}
						else {
							char reply[] = "DOES NOT EXIST";
							write(sockfd, reply, strlen(reply) + 1);
						}
					}
					else if (strcmp("CD", cmd) == 0) {
						if (clientlist[sockfd][2] == 1){
							char *cwd = getenv("PWD");
							char serverCD[MAXLINE] = "cd ";
							char *temp = strrchr(cwd, '/');
	
							if (strcmp(cmd_arg, "..") == 0) {
								*temp = '\0';
								if (chdir(cwd) == 0) {
									char reply[] = "OK";
									write(sockfd, reply, strlen(reply) + 1);
									setenv("PWD", cwd, 1);
								}
								else {
									char reply[] = "FAILED";
									write(sockfd, reply, strlen(reply) + 1);
								}
							}
							else {
								strcat(cwd, "/");
								strcat(cwd, cmd_arg);
								if (chdir(cwd) == 0) {
									char reply[] = "OK";
									write(sockfd, reply, strlen(reply) + 1);
									setenv("PWD", cwd, 1);
								}
								else {
									char reply[] = "FAILED";
									write(sockfd, reply, strlen(reply) + 1);
								}
							}
						}
						else {
                            char reply[] = "NOPASS";
                            write(sockfd, reply, strlen(reply) + 1); 
	                    }
					}
				}
			}
		}
	}
}

#include "include/main.h"
#include "include/otherfunctions.h"
#include "include/webserver.h"

/* ======================================== Main Function Start here ======================================== */
/*   The main function the fd and check for the syntax of the command, it then forks a process that listen    */

int main(int argc, char **argv) {	
	
	int c,port =0,daemon=0, pid, listenfd, socketfd;
	size_t length; char pathbuf[80];
	static struct sockaddr_in client_address; 
	static struct sockaddr_in server_address; 
	char docdir[80],pmode;
	invalid=1;
	/* Check command syntax */
	if (argc > 1 ) {
		if (! checkargs(argv,argc,&port,eventlog,&daemon)) 
			usage();
	}
	/* display parallel mode to be selected */
	do {
		printf("Select the parallel mode:\n");
		printf("\tPress 1 for Fork method.\n");
		printf("\tPress 2 for Pre-Fork method.\n");
		printf("\tPress 0 to exit.\n");
		printf("Enter your choice:");
		scanf("%d",&parallelMode); 
	} while (!(parallelMode == 1 || parallelMode==2 || parallelMode == 0));

	switch(parallelMode){
		case 1: printf("Program started in Fork mode.\n"); break;
		case 2: printf("Program started in Pre-Fork mode.\n"); break;
		case 0: printf("Program exited.\n"); exit(0);
	}

	//printf("port=%d,logfile=%s,daemon=%d\n",port,clflog,daemon); 
	/* check logtype */
	if (!strcmp(eventlog,"")) {
		logtype= LOG2SYS;
	} else logtype= LOG2FILE;
	
	/* Read configuration file */
 	readconfigfile(&port, docdir);

	/* check arguments validity */
	getcwd(pathbuf,80); /* Check Documnet Directory, should not be executable directory */
	if (!strncmp(pathbuf,docdir,strlen(docdir)-1)) {printf("CRITERROR: Cannot use executable directory as document directory\n");exit(0);} 
	if(chdir(docdir) == -1){  printf("CRITERROR: Can't Change directory to %s\n",docdir); exit(0);} /* dir does not exist */
	if (port < 0 || port > 65535) {  printf("Enter port between 1-65535\n"); exit(0);}          /* check port */
	readmime(); /* Read mimi file to get the files types that web can support */

	/* ----------------------------create a child that listen to socket (server)-------------------------------*/
	/* Daemonize the process */
	printf ("Process ID: %d\n", getpid() + 1);
	if(fork() != 0)
		return 0; 
	signal(SIGCLD, SIG_IGN); 
	signal(SIGHUP, SIG_IGN); 
	for(c=0;c<32;c++)close(c);		
		setpgrp();		

	c=open("/dev/null",O_RDWR); dup(c); dup(c); /* handle standart I/O */
	umask(027); /* set newly created file permissions */
	
	logwriter(LOG,"mywebserver starting",argv[2],getpid());
	/* setup the network socket */
	if((listenfd = socket(AF_INET, SOCK_STREAM,0)) <0)
		logwriter(CRITERROR, "Error","Error opening socket",0);
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(port);
	
	if(bind(listenfd, (struct sockaddr *)&server_address,sizeof(server_address)) <0)
		logwriter(CRITERROR,"Error","Error in bind",0);
	if( listen(listenfd,64) <0) 
		logwriter(CRITERROR,"Error","Error in listen",0);
printf("Webserver listening on port:%d\n",port);
	/* Run into the parallel mode selected */
	if (parallelMode==PREFORK) {
		signal(SIGUSR1, spawn);
		for(c = 0; c < MAXPREFORKCHILD; c++)
			forkchild(listenfd);
		while(1) {
		sleep(10000);
		}

	} else {
			/* This process will create a child process as soon as a connection is made */
			while (1) {
				hit++;
				length = sizeof(client_address);
				if((socketfd = accept(listenfd, (struct sockaddr *)&client_address, &length)) < 0)
					logwriter(CRITERROR,"Connection cannot be Established","accept",0);

				if((pid = fork()) < 0) {
					logwriter(CRITERROR,"Child process cannot be created","fork",0);
				}
				else {
					if(pid == 0) { 	/* child */
						close(listenfd);
						webserver(socketfd,hit); 
					} else { 	/* parent */
						close(socketfd);
					}
				}
			} // end of while
		} // end of else
}
/* ======================================== END Main Function  ======================================== */


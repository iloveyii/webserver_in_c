int webserver(int con_fd, int hit);

/* ====================================== find ip in the client request ===================================== */
void findip(char * temp){
	int i=0; char* b = "Host:"; 
	temp=strstr(temp,b) + 6;
	while (!(temp[i]==':')){
		strip[i]=temp[i];
		i++;
	}
	strip[i]='\0'; 
}

/* =========================================== print to syslog ============================================== */
void printsyslog(char *msg) {
     setlogmask (LOG_UPTO (LOG_NOTICE));
     openlog ("MyWebServer", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
     syslog (LOG_NOTICE, "%s %d", msg, getuid ());
     closelog();
}

/* ======================================== Log Function Starts here ======================================== */
void logwriter(int type, char *string1, char *string2, int num){ 
	
	int filed; 
	char logbuffer[BUFSIZE*2];

	switch (type) {
	case CRITERROR: // stop and exit, dont send to web   ,log
		  sprintf(logbuffer,"CRITERROR: %s:%s Errno=%d exiting pid=%d",string1, string2, errno,getpid()); break;
	case SENDSTR: // continue, append this message to the webpage,log
		  sprintf(logbuffer,"USER INFO: %s:%s Error occured=%d exits pid=%d",string1, string2, errno,getpid()); break;
	case LOG:   // continue,  dont send to web  ,log
		  sprintf(logbuffer,"EVENT: %s:%s:%d",string1, string2,num); break;
	case CLF:  // send to CLF
			sprintf(logbuffer,"%s",string1);
			if((filed = open(clflog, O_CREAT| O_WRONLY | O_APPEND,0644)) >= 0) {
			  write(filed,logbuffer,strlen(logbuffer)); 
			  write(filed,"\n",1);      
			  close(filed);
			} else { printf("ERROR: Can't open file %s\n",clflog); exit(0);}
			break;
	}
	// send to client
	if (type==SENDSTR){
		// connectionFD, client request type, request file name(fd) to send, extension of file
		sendResponse(num,"GET","cust1.htm","text/html", hit);
		write(num,logbuffer,strlen(logbuffer)); 
		sendResponse(num,"GET","cust3.htm","text/html", hit);
	} 

	if (logtype==LOG2FILE) {                                                           // log to file
		if (type==CRITERROR || type==SENDSTR || type==LOG) {
			/* log to file eventlog */
			if((filed = open(eventlog, O_CREAT| O_WRONLY | O_APPEND,0644)) >= 0) {
				  write(filed,logbuffer,strlen(logbuffer)); 
				  write(filed,"\n",1);  
				  close(filed);
			} else { logwriter(CRITERROR,"Error","Can't open log file\n",0);} 
		}
	} else { if (type==CRITERROR || type==SENDSTR || type==LOG) printsyslog(logbuffer);} // log to syslog
	if(type == CRITERROR ) exit(0);
}

/* ===================================== Read mimitypes into array mimext =================================== */
int readmime(){
   int r=0; FILE *mimifd;
   char line[80], mimiext[200][50];
   char split1[50], split2[30];

   if (!(mimifd = fopen ("./mime.types", "rt"))) {    printf("ERROR: Can't open file mime.types\n"); exit(0);}
   while(fgets(line, 80, mimifd) != NULL) {
	split(line,split1, split2); strcpy(extensions[r].ext,split2); strcpy(extensions[r].filetype,split1);
	//printf("Filetype =%s, Exten=%s",extensions[r].filetype,extensions[r].ext);
	r++; 
   }
	strcpy(extensions[r].ext,"*"); strcpy(extensions[r].filetype,"*");
   fclose(mimifd);
   return 1;
}

/* ======================================== to split string on space ======================================== */
split(char *original, char * split1, char * split2) {
  int i = 0;  int j = 0;  int flag = 0;int k=0;
  while (original[i] != '\0'){
     if (flag == 0) {
        if (original[i] == ' '){flag = 1;split1[i] = '\0';}
     	else {split1[k++] = original[i];}
     }
     else {split2[j++] = original[i];}
     i++;
  }
  split2[j] = '\0';
}

/* ================================== to display the usage of the command =================================== */
usage () {
  if (invalid) {  printf("\tInvalid arguments.\n");}
    printf("\tThe syntax of the command is:\n\tSyntax: ./mywebserver [-p port] [-l logfile] [-d] [-?]\n");
    printf("\toption -p: The port number must be between 1 and 65535.\n");
    printf("\toption -l: Enter a valid Event log file name.\n");
    printf("\toption -d: To run the server as a daemon.\n");
    printf("\toption -?: To show this help menu.\n");
  exit(0);
}

/* =============================================== get head info =========================================== */
getheadinfo(char *buff,char *fn,char *ftype){
	char timeStr[100] = ""; 
	char tmpbuff[400]="";
	struct stat fstats;
	time_t now;  
	time(&now);strftime(timeStr, 100, "%d-%m-%Y %H:%M:%S", localtime( &fstats.st_mtime));
	if (!stat(fn, &fstats)){
		sprintf(tmpbuff,"HTTP/1.0 200 OK\nServer: mywebserver 13oct2010\nContent-Type: %s\nDate: %.24s GMT\nLast-Modified: Tue, %s GMT\nAccept-Ranges: bytes\nConnection: close\nContent-Length: %ld\r\n",ftype,ctime(&now), timeStr,fstats.st_size);
    }	else { sprintf(tmpbuff,"error getting atime\r\n");}
	strncat(buff,tmpbuff,strlen(tmpbuff));
	return 0;
}

/* ======================================= check command line arguments ==================================== */
int checkargs(char **argv,int argc, int *port, char *eventlog,int *daemon) {
	int flag,c;
	if( argc > 6 ) {
			usage();
			exit(0);
		}
		/* assign arguments to variables */
		for (c=1;c< argc;c++){
			if (!strcmp(argv[c],"-p")) { if(c+1<argc) {*port=atoi(argv[c+1]);flag=1;} else {flag=0;break;}}
			if (!strcmp(argv[c],"-l")) { if(c+1<argc) {strcpy(eventlog,argv[c+1]);flag=1;} else {flag=0;break;}}
			if (!strcmp(argv[c],"-d")) {*daemon=1;flag=1;} 
			if (!strcmp(argv[c],"-?")) {flag=0;invalid=0;} 
		}
	if (flag) 
		return 1;
	else
		return 0;
}

/* ====================================== Read configuration file ==================================== */
int readconfigfile(int *port, char * ddir) {
	FILE *configfile; char line[80];
	/* Read configuration file */
 	if  (!( configfile = fopen (".lab3-config", "rt")))
		{  printf("ERROR: Can't open file .lab3-config\n"); exit(0);};  /* open the file for reading */
   	fgets(line, 80, configfile); strcpy(ddir,line); ddir[strlen(ddir)-1]='\0'; 
	if (*port == 0) {fgets(line, 80, configfile); *port=atoi(line);}
	fclose(configfile);
	  printf("conf doc dir=%s\tconf port=%d\n",ddir,*port); 
}

/* ================================ to check if an extension is supported ============================= */
int checkExtSupport(char buffer[]) {
	char *extStr = (char *)0; int i, extLen, buflen;
	buflen=strlen(buffer);
	for(i=0;strncmp(extensions[i].ext,"*",1);i++) {
		extLen = strlen(extensions[i].ext);  
		if( !strncmp(&buffer[buflen-extLen+1], extensions[i].ext, extLen - 1)) {
			extStr =extensions[i].filetype; 
			break;
		} 
	}
	if (!strncmp(extensions[i].ext,"*",1)) {logwriter(LOG,"webserver does not supported extension: ",extStr,1); return UNSUCCESSFUL;}
	return SUCCESSFUL;
}

/* ================================== to send response to the client ================================= */
int sendResponse(int con_fd,char *reqtype,char *reqfile,char * extStr,int hit){
	static char buffer[BUFSIZE+1]; int ret,statuscode;int reqfile_fd;

// check for file not found and return related page: Not found	
	if(( reqfile_fd = open(reqfile,O_RDONLY)) == -1) { 
		statuscode=400; strcpy(extStr,"text/html");
		if(( reqfile_fd = open("400.htm",O_RDONLY)) == -1);
	} else {statuscode=200;}
	/* send the header to the client*/
	sprintf(buffer,"HTTP/1.0 200 OK\r\nContent-Type: %s\r\n\r\n", extStr);
	write(con_fd,buffer,strlen(buffer));

	/* GET or HEAD */
	if (!strncmp(reqtype,"GET",strlen(reqtype))) { /* do get */		
		while (	(ret = read(reqfile_fd, buffer, BUFSIZE)) > 0 ) {
			  write(con_fd,buffer,ret);
		}
	}
	if (!strncmp(reqtype,"HEAD",strlen(reqtype))) { /* do head */
		/* getheadinfo */
		getheadinfo(buffer,reqfile,extStr);
		write(con_fd,buffer,strlen(buffer));	
	}
	if (statuscode == 200) {logwriter(LOG,"Response: sent file ",reqfile,hit);} else {logwriter(LOG,"Response: sent file ","400.htm",hit);}
	return statuscode;
}

/* ================================== for prefork  ================================= */
void forkchild(int sd) {
	int fd, pid; 

	pid = fork();
	if(pid == 0) {
		while(1) {
			fd = accept(sd, NULL, NULL);
			webserver(fd,hit++);
			close(fd);
		}
	} 
}
int gsd; 
void spawn(int sig) {
	forkchild(gsd);
}


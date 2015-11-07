/* ======================================== Webserver Function Starts here ======================================== */
int webserver(int con_fd, int hit) {
	int status, statuscode=0;
	long i,j,ret,ret2=0,reqfile_fd; 
	char tmpbuff[200]; char requesturl[100]; char file2open[50];
	char extStr[40]; char reqfile[50],reqtype[5];
	static char buffer[BUFSIZE+1];static char tbuffer[BUFSIZE+1];
	time_t now; 
	struct stat fstats;
	
	/* read the request of client in buffer */
	ret =read(con_fd,buffer,BUFSIZE); 
	if(ret > READBUFSIZE ) {
		logwriter(SENDSTR,"Length of requested url is too long.",tbuffer,con_fd);
		return 0;
	}

	if(ret == 0 || ret == -1) {	
		logwriter(LOG,"failed to read browser request","",con_fd);
	}

	if(ret > 0 && ret < BUFSIZE)	
		buffer[ret]=0;		
	else buffer[0]=0;

	/* save the url for future */
	strncpy(requesturl,buffer,100);

		for(i=0;i<ret;i++)	
		if(buffer[i] == '\r' || buffer[i] == '\n')
			buffer[i]='*';
	logwriter(LOG,"Request:",buffer,hit);
	
	/* check the request type */
	if(strncmp(buffer,"GET ",4) && strncmp(buffer,"get ",4) )
		if( strncmp(buffer,"HEAD ",4) && strncmp(buffer,"head ",4) ) 
			{logwriter(SENDSTR,"Only simple GET/HEAD operation supported","0",con_fd); return 0;}
		else strcpy(reqtype,"HEAD");
	else strcpy(reqtype,"GET");
	
	/* Formate for substring extraction */
	for(i=4;i<BUFSIZE;i++) { 
		if(buffer[i] == ' ') { 
			buffer[i] = 0;
			break;
		}
	}
	/* check for lenghty URLS	*/
	if(strlen(&buffer[5]) > READBUFSIZE) {
		
		return 0;
	}

	/* check for URL Validation */
	for(j=0;j<i-1;j++) 	
		if(buffer[j] == '.' && buffer[j+1] == '.') {
			logwriter(SENDSTR,"Path traversal (..) not supported","a",con_fd);
			return 0;
			break;
		}
	/* set default page */
	if( !strncmp(&buffer[0],"GET /\0",6) || !strncmp(&buffer[0],"get /\0",6) ) /* send index file if no file specified */
		  strcpy(buffer,"GET /index.html");
	if( !strncmp(&buffer[0],"HEAD /\0",7) || !strncmp(&buffer[0],"head /\0",7) ) /* send index file if no file specified */
		  strcpy(buffer,"HEAD /index.html");
	strcpy(reqfile,&buffer[5]); /* save file name */

   /* check if we support the extension if not display proper page */
	status = checkExtSupport(buffer);
	
	/* check for http status codes */
	// check for: set the requested page; default vals
	strcpy(file2open,&buffer[5]); statuscode=200;
	// check for not supported extension and return related page : Not implemented.
	if (status == UNSUCCESSFUL) {strcpy(file2open,"501.htm"); statuscode=501;}

	/* Send response to the client */
	// connectionFD, client request type, request file name(fd) to send, extension of file
	statuscode= sendResponse(con_fd,reqtype,file2open,extStr, hit);
	
	/* log to common log format */
	findip(requesturl); time(&now); stat(file2open, &fstats);i=fstats.st_size; if (statuscode==400) i=0;
	sprintf(tmpbuff,"%s - - [%.24s GMT] %s /%s HTTP/1.0 %d %ld",strip, ctime(&now),reqtype,reqfile,statuscode,i);
	logwriter(CLF,tmpbuff,strip,getpid());
	if ( parallelMode == FORK) 
		exit(1);
}

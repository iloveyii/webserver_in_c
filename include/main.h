#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/stat.h>
#include <syslog.h>

#define BUFSIZE 8196
#define READBUFSIZE 8000 /* 15 chars when 400 */
#define CRITERROR 42
#define SORRY 10
#define LOG   20
#define CLF   30
#define SENDSTR 40
#define LOG2FILE 1
#define LOG2SYS 2
#define GET 1
#define HEAD 2
#define SUCCESSFUL 1
#define UNSUCCESSFUL 0
#define FORK 1
#define PREFORK 2
#define MAXPREFORKCHILD 3
/* ========================================     GLobal Variables   ======================================== */
struct {
	char ext[10];
	char filetype[60];
} static extensions [700];
char clflog[10] ="clf.log";
char eventlog[]="";
char strip[100];
int invalid=1, logtype, parallelMode,hit;

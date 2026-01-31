
#include "common.h"
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
static void usage(const char *a){fprintf(stderr,"Usage: %s <pid>\n",a); exit(1);}
static int isnum(const char*s){for(;*s;s++) if(!isdigit(*s)) return 0; return 1;}
int main(int c,char**v){
 if(c!=2||!isnum(v[1])) usage(v[0]);
 printf("PID:%s\n", v[1]);
    char statpath[256];
    snprintf(statpath, sizeof(statpath), "/proc/%s/stat", v[1]);

    FILE *sf = fopen(statpath, "r");
    if(!sf){
	if(errno == ENOENT) fprintf(stderr, "PID not found\n");
	else if(errno == EACCES) fprintf(stderr, "Permission denied\n");
	else perror("fopen statw");
	return 1;
    }
    char line[8192];
    if(!fgets(line, sizeof(line), sf)){ fclose(sf); return 1; }
    fclose(sf);

    int pid_i = 0, ppid = -1;
    char comm[256];
    char state = '?';

    if(sscanf(line, "%d (%255[^)]) %c %d", &pid_i, comm, &state, &ppid) != 4) return 1;

    printf("State:%c\n", state);
    printf("PPID:%d\n", ppid);

    char *rp = strrchr(line, ')');
    if(!rp || rp[1] != ' ') return 1;

    char *rest = rp + 2;
    char *save = NULL;
    char *tokens[128];
    int nt = 0;

    for(char *tok = strtok_r(rest, " \n", &save); tok && nt < 128; tok = strtok_r(NULL, " \n", &save)){
        tokens[nt++] = tok;
    }
    if(nt <= 36) return 1;

    long long utime = atoll(tokens[11]);
    long long stime = atoll(tokens[12]);
    int processor = atoi(tokens[36]);

    long ticks = sysconf(_SC_CLK_TCK);
    double cpu_sec = 0.0;
    if(ticks > 0) cpu_sec = (double)(utime + stime) / (double)ticks;

    printf("CPU:%d %.3f\n", processor, cpu_sec);

    char cmdpath[256];
    snprintf(cmdpath, sizeof(cmdpath), "/proc/%s/cmdline", v[1]);

    FILE *cf = fopen(cmdpath, "rb");
    if(!cf){
        printf("Cmd:%s\n", comm);
    } else {
        char buf[4096];
        size_t n = fread(buf, 1, sizeof(buf) - 1, cf);
        fclose(cf);

        if(n == 0){
            printf("Cmd:%s\n", comm);
        } else {
            buf[n] = '\0';
            for(size_t i=0;i<n;i++) if(buf[i]=='\0') buf[i] = ' ';
            while(n>0 && (buf[n-1]==' ' || buf[n-1]=='\n' || buf[n-1]=='\0')) { buf[n-1]='\0'; n--; }
            printf("Cmd:%s\n", buf);
        }
    }

    char statuspath[256];
    snprintf(statuspath, sizeof(statuspath), "/proc/%s/status", v[1]);

    FILE *f = fopen(statuspath, "r");
    if(!f) {
	if(errno == ENOENT) fprintf(stderr, "PID not found\n");
	else if (errno == EACCES) fprintf(stderr, "Permission denied\n");
	else perror("fopen status");
	return 1;
    }
    char sline[512];
    long rss_kb = 0;

    while(fgets(sline, sizeof(sline), f)){
        if(strncmp(sline, "VmRSS:", 6) == 0){
            sscanf(sline, "VmRSS:%ld", &rss_kb);
            break;
        }
    }
    fclose(f);

    printf("VmRSS:%ld\n", rss_kb);
    return 0;
}

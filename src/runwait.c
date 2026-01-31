#include "common.h"
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
static void usage(const char *a){fprintf(stderr,"Usage: %s <cmd> [args]\n",a); exit(1);}
static double d(struct timespec a, struct timespec b){
 return (b.tv_sec-a.tv_sec)+(b.tv_nsec-a.tv_nsec)/1e9;}
int main(int c,char**v){
 if (c < 2) usage(v[0]);

 struct timespec t0, t1;
 clock_gettime(CLOCK_MONOTONIC, &t0);

 pid_t pid = fork();

 if (pid == 0) {
     execvp(v[1], &v[1]);
     _exit(127);
 }

 int status = 0;
 waitpid(pid, &status, 0);

 clock_gettime(CLOCK_MONOTONIC, &t1);

 double elapsed = d(t0, t1);

 if (WIFEXITED(status)) {
       printf("pid=%d elapsed=%.3f exit=%d\n", (int)pid, elapsed, WEXITSTATUS(status));
 } else if (WIFSIGNALED(status)) {
       printf("pid=%d elapsed=%.3f signal=%d\n", (int)pid, elapsed, WTERMSIG(status));
 } else {
       printf("pid=%d elapsed=%.3f exit=?\n", (int)pid, elapsed);
 }

 return 0;
}

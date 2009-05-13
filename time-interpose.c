/*
 * Author: patrick@collison.ie
 * Public domain
 */

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PID_LEN 5
#define TICK 500000 /* half a second */

struct interpose {
  void *new;
  void *old;
};

int ogettimeofday(struct timeval *, struct timezone *);

static const struct interpose interposers[] __attribute__ ((section("__DATA, __interpose"))) = {{
  (void *) ogettimeofday, (void *) gettimeofday
}};

static int t_offset;
static int t_offset_defined = 0;

char *offset_file_path() {
  char *path, *base = "/tmp/offset.";
  path = malloc((strlen(base) + MAX_PID_LEN + 1) * sizeof(char));
  if(path) {
    sprintf(path, "%s%d", base, getpid());
    return path;
  } else {
    fprintf(stderr, "Couldn't malloc");
    exit(-1);
  }
}

int read_offset(char *path) {
  FILE *fp = fopen(path, "r");
  if(fp) {
    if(fscanf(fp, "%d", &t_offset) == 1) {
      return 1;
    }
    fclose(fp);
  }
  return 0;
}

void get_offset() {
  char *path = offset_file_path();
  while(!read_offset(path)) usleep(TICK);
  t_offset_defined = 1;
  free(path);
}

int ogettimeofday(struct timeval *tp, struct timezone *tzp) {
  int ret;

  if(!t_offset_defined) get_offset();

  ret = gettimeofday(tp, tzp);
  tp->tv_sec += t_offset;
  return ret;
}

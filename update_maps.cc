#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <iostream>


static void
proc_setgroups_write(pid_t child_pid, char *str)
{
    char setgroups_path[PATH_MAX];
    int fd;

    snprintf(setgroups_path, PATH_MAX, "/proc/%ld/setgroups",
            (long) child_pid);

    fd = open(setgroups_path, O_RDWR);
    if (fd == -1) {
        if (errno != ENOENT)
            fprintf(stderr, "ERROR: open %s: %s\n", setgroups_path,
                strerror(errno));
        return;
    }

    if (write(fd, str, strlen(str)) == -1)
        fprintf(stderr, "ERROR: write %s: %s\n", setgroups_path,
            strerror(errno));

    close(fd);
}

static void
update_map(char *mapping, char *map_file)
{
    int fd, j;
    size_t map_len;     /* Length of 'mapping' */

    /* Replace commas in mapping string with newlines */

    map_len = strlen(mapping);
    for (j = 0; j < map_len; j++)
        if (mapping[j] == ',')
            mapping[j] = '\n';

    fd = open(map_file, O_RDWR);
    if (fd == -1) {
        fprintf(stderr, "ERROR: open %s: %s\n", map_file,
                strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (write(fd, mapping, map_len) != map_len) {
        fprintf(stderr, "ERROR: write %s: %s\n", map_file,
                strerror(errno));
        exit(EXIT_FAILURE);
    }

    close(fd);
}

void new_user_maps(pid_t child_pid){
	int flags, opt, map_zero;
	
    //struct child_args args;
	char *uid_map, *gid_map;
    const int MAP_BUF_SIZE = 100;
    char map_buf[MAP_BUF_SIZE];
    char map_path[PATH_MAX];
    gid_map = "65534 1";
    uid_map = "65534 1";
    map_zero = 1;


	//update maps
	std::cout <<"Update maps " <<std::endl;
	if (uid_map != NULL || map_zero) {
		std::cout <<"I am in update-map-function" <<std::endl;
        snprintf(map_path, PATH_MAX, "/proc/%ld/uid_map",
                (long) child_pid);
        if (map_zero) {
            snprintf(map_buf, MAP_BUF_SIZE, "0 %ld 1", (long) getuid());
            uid_map = map_buf;
        }
        update_map(uid_map, map_path);
    }

    if (gid_map != NULL || map_zero) {
        proc_setgroups_write(child_pid, "deny");

        snprintf(map_path, PATH_MAX, "/proc/%ld/gid_map",
                (long) child_pid);
        if (map_zero) {
            snprintf(map_buf, MAP_BUF_SIZE, "0 %ld 1", (long) getgid());
            gid_map = map_buf;
        }
        update_map(gid_map, map_path);
    }

}

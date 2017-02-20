#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <linux/unistd.h>
#include <iostream>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <dirent.h>


#define check(ret) \
  do { \
    if ((ret) == -1) { \
      std::cerr << __FILE__ \
        << ':' << __LINE__ \
        << ':' << __func__ \
        << ' ' << strerror(errno) \
        << std::endl; \
      exit(1); \
    } \
  } while (0)

static void proc_setgroups_write(pid_t child_pid, char *str);
static void update_map(char *mapping, char *map_file);
void new_user_maps(pid_t child_pid);


pid_t child_pid;
int  pipe_fd[2];

void load_gcc(){

	std::cout <<"Load gcc compiler..." <<std::endl;
	system(
		"yum --installroot=/home/svetasvesh/mnt/new_root \
		--releasever=7 \
		--nogpgcheck \
		-y \
		install gcc.x86_64"
	);
	system(
		"yum --installroot=/home/svetasvesh/mnt/new_root \
		--releasever=7 \
		--nogpgcheck \
		-y \
		install gcc-c++.x86_64"
	);

	system(
		"yum --installroot=/home/svetasvesh/mnt/new_root \
		--releasever=7 \
		--nogpgcheck \
		-y \
		install wget.x86_64"
	);
	system(
		"yum --installroot=/home/svetasvesh/mnt/new_root \
		--releasever=7 \
		--nogpgcheck \
		-y \
		install zlib-devel.x86_64"
	);
	std::cout <<"Load make..." <<std::endl;
	system(
		"yum --installroot=/home/svetasvesh/mnt/new_root \
		--releasever=7 \
		--nogpgcheck \
		-y \
		install make.x86_64"
	);
	// system(
	// 	"yum --installroot=/home/svetasvesh/mnt/new_root \
	// 	--releasever=7 \
	// 	--nogpgcheck \
	// 	-y \
	// 	install mercurial"
	// );
}

void load_uuid_lib() {
	std::cout <<"Load ..." <<std::endl;
}

void change_fs() {

	std::cout <<"Change filesystem..." <<std::endl;
	const char* dirpath = "/lib";
	const char* put_old = "/home/svetasvesh/mnt/new_root/put_old";
	const char* new_root = "/home/svetasvesh/mnt/new_root";
	const char* project_path = "/put_old/mnt/home/svetasvesh/collector/spec-factory";
	

	check(chdir(new_root));
	check(mount("", "/", "none", MS_PRIVATE | MS_REC, NULL));
	
	check(mount("", "/proc", "proc", MS_NOEXEC | MS_NOSUID | MS_NODEV, NULL));
	check(mount("", "/dev", "tmpfs", MS_NOSUID | MS_STRICTATIME, "mode=755"));
	check(system("mkdir /home/svetasvesh/mnt/new_root/tmp/spec-factory"));
	


	check(mount(new_root, new_root, "none", MS_BIND | MS_REC, NULL));
	check(syscall(SYS_pivot_root, new_root, put_old));
	std::cout <<"/put_old/mnt/home/svetasvesh/collector/spec-factory:" <<std::endl;
	check(system("ls -la /put_old/mnt/home/svetasvesh/collector/spec-factory"));
	check(mount(project_path, "/tmp/spec-factory", "none",  MS_BIND, NULL));

		
}

void make_lib() {
	std::cout <<"Make lib..." <<std::endl;
	std::cout <<"/tmp:" <<std::endl;
	check(system("ls -la /tmp"));
	check(chdir("/tmp/spec-factory/"));
	std::cout <<"/tmp/spec-factory:" <<std::endl;
	check(system("ls -la"));
	system("make");
	system("cd test;\
		../bin/discovery server localhost:10001 &\
		../bin/discovery server localhost:10002 &\
		../bin/discovery client localhost:10000");
	check(umount2("/put_old", MNT_DETACH));

}

int load_env(void* pid_size) {

	char ch;

	close(pipe_fd[1]);

	if (read(pipe_fd[0], &ch, 1) == 0) {
        fprintf(stderr, "Failure in child: read from pipe returned != 0\n");
        exit(EXIT_FAILURE);
    }

	//load_gcc();
	change_fs();
	//load_uuid_lib();
	make_lib();
	exit(0);
}

void create_env() {
	size_t stack_size = 1024*1024;
	char* child_stack = new char[stack_size];
	void* child_stack_end = child_stack + stack_size;

	check(child_pid = clone(load_env, child_stack_end, CLONE_NEWPID | CLONE_NEWUSER | CLONE_NEWNS | SIGCHLD, 0));
	new_user_maps(child_pid);
	close(pipe_fd[1]);

	int status = 0;
  	check(waitpid(child_pid, &status, 0));

}

int main() {
	create_env();
}
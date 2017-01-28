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
		install gcc-gfortran.x86_64"
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
		install tar.x86_64 gzip.x86_64"
	);
	std::cout <<"Load make..." <<std::endl;
	system(
		"yum --installroot=/home/svetasvesh/mnt/new_root \
		--releasever=7 \
		--nogpgcheck \
		-y \
		install make.x86_64"
	);
}

void load_uuid_lib() {
	std::cout <<"Load uuid lib..." <<std::endl;
	system("ls -l /dev/null");
	system("rm -rf /tmp");
	system("mkdir /tmp");
	check(chdir("/tmp"));
	//system("wget http://www.mirrorservice.org/sites/ftp.ossp.org/pkg/lib/uuid/uuid-1.6.2.tar.gz");
	system("wget http://www.netlib.org/blas/blas-3.7.0.tgz");
	system ("tar -xvzf blas-3.7.0.tgz");
}

void change_fs() {

	std::cout <<"Change filesystem..." <<std::endl;
	const char* dirpath = "/lib";
	const char* put_old = "/home/svetasvesh/mnt/new_root/put_old";
	const char* new_root = "/home/svetasvesh/mnt/new_root";
	

	check(chdir(new_root));
	check(mount("", "/", "none", MS_PRIVATE | MS_REC, NULL));
	
	check(mount("", "/proc", "proc", MS_NOEXEC | MS_NOSUID | MS_NODEV, NULL));
	//check(mount("", "/dev", "devtmpfs", MS_NOSUID | MS_STRICTATIME, "mode=755"));
	check(mount("/dev", "/home/svetasvesh/mnt/new_root/dev", "none", MS_BIND | MS_REC, NULL));

	//system("/var/tmp/mount_dev");


	check(mount(new_root, new_root, "none", MS_BIND | MS_REC, NULL));
	check(syscall(SYS_pivot_root, new_root, put_old));

	/*DIR* dir = opendir(dirpath);
	std::cout <<"I open dir " <<dir <<std::endl;
	dirent* dirp;
	while ( (dirp=readdir(dir)) != NULL) {
		std::cout <<"I read dir " <<dirp->d_name <<std::endl;
	}*/
	//check(closedir(dir));

	check(umount2("/put_old", MNT_DETACH));
	//system("cp -f /put_old/etc/resolv.conf /etc/resolv.conf");
	//system("ln -sfL /put_old/proc /proc");
	//system("ln -sfL /put_old/sys /sys");
	//system("ln -sfL /put_old/dev /dev");

//	system("id");
	//check(mount("/put_old/sys", "/sys", "sysfs", 0, NULL));
	

	
}

void make_lib() {
	std::cout <<"Make lib..." <<std::endl;
	check(chdir("/tmp/BLAS-3.7.0"));
	system("ls -l /dev/null");
	//system("./configure \
	//	--prefix=/tmp/uuid");
	system("make");
	system("make check");
}

int load_env(void* pid_size) {

	char ch;

	close(pipe_fd[1]);

	if (read(pipe_fd[0], &ch, 1) == 0) {
        fprintf(stderr, "Failure in child: read from pipe returned != 0\n");
        exit(EXIT_FAILURE);
    }

	load_gcc();
	change_fs();
	load_uuid_lib();
	make_lib();
	exit(0);
}

void create_env() {
	size_t stack_size = 1024*1024;
	char* child_stack = new char[stack_size];
	void* child_stack_end = child_stack + stack_size;
	//int pid  = 0;

	check(child_pid = clone(load_env, child_stack_end, CLONE_NEWPID | CLONE_NEWUSER | CLONE_NEWNS | SIGCHLD, 0));
	new_user_maps(child_pid);
	close(pipe_fd[1]);

	int status = 0;
  	check(waitpid(child_pid, &status, 0));

}

int main() {
	create_env();
}
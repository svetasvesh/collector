#define _GNU_SOURCE

#include <unistd.h>
#include <sys/mount.h>


int main() {
	return mount("", "/dev", "devtmpfs", MS_NOSUID | MS_STRICTATIME, "mode=755");
}
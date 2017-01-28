all:
	g++ main.cc update_maps.cc -o collector
	g++ mount_dev.cc -o mount_dev
clean:
	rm -rf *.o collector
	rm -rf *.o mount_dev


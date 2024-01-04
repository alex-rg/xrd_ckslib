INCLUDE=-I/usr/include/xrootd -I/home/vagrant/xrd_src/src
libXrdCksPlugin.so:
	gcc -fPIC -g -c $(INCLUDE) -o libXrdCksPlugin.o XrdCksPlugin.cc
	gcc -shared -g -o libXrdCksPlugin.so libXrdCksPlugin.o
clean:
	rm libXrdCksPlugin.*

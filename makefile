defenddns: conf.h  daemon.h  epoll.h  listen.h  main.h  process.h  process_worker.h  resolver.h  signal.h
	g++ -g -O0 -o defenddns -Wall daemon.cpp  epoll.cpp  listen.cpp  main.cpp  process.cpp  process_worker.cpp  resolver.cpp  signal.cpp
	
clean:
	rm *.o defenddns
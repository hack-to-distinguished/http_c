threadpoolserver: threaded_server_src/http.c threaded_server_src/threadpool.c threaded_server_src/threadpoolserver.c
	gcc -o threadpoolserver threaded_server_src/http.c threaded_server_src/threadpool.c threaded_server_src/threadpoolserver.c 

clean:
	rm -f threadpoolserver 

echoserver:
	# 编译, -ltinymuduo对pthread库有依赖, lpthread放在后面
	g++ -o echoserver echoserver.cc -std=c++17 -ltinymuduo -lpthread -g
	g++ -o async-echoserver async-echoserver.cc -std=c++17 -ltinymuduo -lpthread -g
	g++ -o async-httpserver async-httpserver.cc -std=c++17 -ltinymuduo -lpthread -g
clean:
	rm -f echoserver
	rm -f async-echoserver
	rm -f async-httpserver
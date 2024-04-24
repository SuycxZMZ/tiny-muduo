echoserver:
	# 编译, -ltinymuduo对pthread库有依赖, lpthread放在后面
	g++ -o echoserver echoserver.cc -std=c++17 -ltinymuduo -lpthread -g -I ../include
clean:
	rm -f echoserver
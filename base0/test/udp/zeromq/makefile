all:
	g++ -g -Wall -o publisher publisher.cpp -I/opt/third_party/zeromq/include -L/opt/third_party/zeromq/lib -lzmq
	g++ -g -Wall -o subscriber subscriber.cpp -I/opt/third_party/zeromq/include -L/opt/third_party/zeromq/lib -lzmq

clean:
	rm -rf publisher subscriber

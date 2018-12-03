all:
	g++ -g -Wall -o sender sender.cpp -I/opt/third_party/boost/include -L/opt/third_party/boost/lib -lboost_system
	g++ -g -Wall -o receiver receiver.cpp -I/opt/third_party/boost/include -L/opt/third_party/boost/lib -lboost_system

clean:
	rm -rf sender receiver

CXX = g++
CFLAGS = -std=c++14 -O2 -Wall -g

TARGET = server

OBJS = server

all: $(OBJS) 
	$(CXX) $(CFLAGS) -o $(TARGET) -pthread -lmysqlclient ../lib/libjsoncpp.a
CXX = g++
CFLAGS = -std=c++14 -O2 -Wall -g 

TARGET = server
OBJS = ../main.cpp ../net/*.cpp ../HTTP/*.cpp ../sql/*cpp ../base/*.cpp

all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o ../bin/$(TARGET)  -pthread -lmysqlclient ../lib/libjsoncpp.a

clean:
	rm -rf ../bin/$(OBJS) $(TARGET)





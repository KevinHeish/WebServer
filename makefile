CXX=g++
CFLAGS = -std=c++17 -Wall
CFLAGS += -I./inc/ -I/usr/include/mysql
CFLAGS += -L/usr/local/lib
TARGET= server.out
OBJS = ./main.cc ./src/Http/*.cc ./src/Buffer/*.cc ./src/WebServer/*.cc ./src/Timer/*.cc ./src/Pool/*.cc 

DEBUG ?=1
ifeq ($(DEBUG),1)
	CFLAGS +=-DDEBUG -g3
else
    	CFLAGS +=-DNDEBUG -O2
endif

all: $(OBJS)
	$(CXX) $(CFLAGS) $(OBJS) -o ./$(TARGET) -pthread -lmysqlpp -lmysqlclient
lib: $(OBJS)
	$(CXX) -fPIC $(CFLAGS) -c $(OBJS)
	$(CXX) -shared $(CFLAGS) *.o -o liba.so

clean:
	rm *.o

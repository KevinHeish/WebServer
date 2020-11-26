# WebServer
Simple HTTP server under Ubuntu handles POST and GET method.    


## C++ HTTP server practice  

Epoll and file descriptor handle write and read events with ET mode.  
Threadpool and non-blocking mode fd to handle concurrency requests.  
Log-in and register user account with MySQL DB.  
Use timer to remove timeout node.  
Concurrency test : Apache benchtest  

![image](https://github.com/KevinHeish/WebServer/blob/main/others/ApacheBenchtest.JPG)

## Environment  
MySQL DB  
Ubuntu 18.04  
C++14 supported compiler  


## Build  
Compiler optimize build  
```
make DEBUG=0
```
DEBUG mode build
```
make
```
lib.so for test code use
```
make lib
```

## ToDo  
- [ ] More test code
- [ ] Try Doublely Linked list in Timer update
- [ ] Put and Delete method
- [ ] Save login state


# Reference and Acknowledgement  

https://github.com/qinguoyi/TinyWebServer  
https://github.com/markparticle/WebServer

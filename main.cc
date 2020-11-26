#include <signal.h>
#include <unistd.h>

#include <thread>

#include <WebServer/Server.h>

Server g_server(8888, 2, 30000, false, 3306, "root", "12345678", "server_test",12,8 );
void handler(int num){
   g_server.set_close(); 
   std::cout << "Recieve SIGNAL STOP , Close thread\n";
}

int main(int argc, char** argv) {
    signal(SIGTSTP,handler);
    g_server.start();
    std::cout << "BYE\n";
    return 0;
}

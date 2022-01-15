/**********************************************************************************************************************/
/*
*Sample code LED , could be used for multithreaded app , device health status monitoring etc
*
* Compile:
* g++ health-status-led-sample.cpp -lpthread -o led
*
*
*/
/**********************************************************************************************************************/


#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<iostream>
#define on 1
#define off 0

void gpio_write(int fd, int value);
void* led_Flash(void* args);


class PThread {
    public:

    pthread_t threadID;
    volatile int suspended;
    int fd;
    pthread_mutex_t lock;
    PThread(int fd1)
   {   
        this->fd=fd1; 
        this->suspended =1;  //Initial state: suspend blinking untill resume call 
        pthread_mutex_init(&this->lock,NULL); 
        pthread_create(&this->threadID, NULL, led_Flash, (void*)this );

    }
    ~PThread() 
    { 
      pthread_join(this->threadID , NULL);
      pthread_mutex_destroy(&this->lock);
    }

    void suspendBlink() {
        pthread_mutex_lock(&this->lock);
        this->suspended = 1;
        pthread_mutex_unlock(&this->lock);
    }

    void resumeBlink() {
        pthread_mutex_lock(&this->lock);
        this->suspended = 0;
        pthread_mutex_unlock(&this->lock);
    }
};

void gpio_write(int fd, int value)
{
if(value!=0)
 printf("%d: on\n", fd);
else
 printf("%d: off\n", fd);
}


void* led_Flash(void* args)
{  
    PThread* pt= (PThread*) args;
    int fd= pt->fd;

    while(1)
    {
    if(!(pt->suspended))
        {
        gpio_write(fd,on);
        usleep(1); 
        gpio_write(fd,off);
        usleep(1);
        }
   }


return NULL;
}


int main()
{
   //Create threads with Initial state: suspend/stop blinking untill resume call 
    class PThread redLED(1);
    class PThread amberLED(2);
    class PThread greenLED(3);

    // Start blinking
    redLED.resumeBlink();
    amberLED.resumeBlink();
    greenLED.resumeBlink();
    sleep(5);

    // suspend/stop blinking
    amberLED.suspendBlink();

    sleep(5);

    redLED.suspendBlink();

    sleep(5);

    amberLED.suspendBlink();

    sleep(5);     

    redLED.resumeBlink();  


pthread_exit(NULL);

return 0;
}



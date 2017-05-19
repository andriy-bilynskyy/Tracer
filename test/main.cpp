#include <iostream>
#include "safeout.h"
#include <unistd.h>
#include <pthread.h>

using namespace std;

void * foo(void *)
{
    for(;;)
    {
        dbg::sout << "worker: " << 365 << dbg::endl;
        usleep(110000);
    }
    return NULL;
}


int main()
{
    pthread_t thr;
    (void)pthread_create(&thr, NULL, foo, NULL);
    for(int i = 0; i < 100; i++)
    {
        dbg::sout << "main: " << 123 << dbg::endl;
        usleep(100000);
    }
    (void)pthread_cancel(thr);
    (void)pthread_join(thr, NULL);
    return 0;
}





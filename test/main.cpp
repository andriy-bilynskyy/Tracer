#include <iostream>
#include "safeout.h"
#include <unistd.h>
#include <pthread.h>

using namespace std;

void * foo(void *)
{
    dbgLevel(dbg::DBG_ERROR);
    for(;;)
    {
        dbg::sout << "worker: " << dbg::dbg << 123 << dbg::err << __DBG_LOCATION__ << " "
                                                               << 365 << dbg::endl;
        usleep(110000);
    }
    return NULL;
}


int main()
{
    dbgLevel(dbg::DBG_ERROR);
    pthread_t thr;
    (void)pthread_create(&thr, NULL, foo, NULL);
    (void)pthread_setname_np(thr, "worker");
    (void)pthread_setname_np(pthread_self(), "main");
    for(int i = 0; i < 100; i++)
    {
        dbg::sout << "main: " << 123 << dbg::endl;
        dbg::sout << dbg::info << "test " << dbg::endl;
        usleep(100000);
    }
    (void)pthread_cancel(thr);
    (void)pthread_join(thr, NULL);
    return 0;
}





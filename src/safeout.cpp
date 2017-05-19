#include "safeout.h"
#include <iostream>
#include <sys/time.h>
#include <time.h>
#include <iomanip>

namespace dbg
{
    safeout::safeout() : m_pos(&std::cout), m_outData()
    {
        pthread_mutexattr_t attr;
        (void)pthread_mutexattr_init(&attr);
        (void)pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST);
        (void)pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
#ifdef DBG_LOG_MULTUPROCESS
        (void)pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
#else
        (void)pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_PRIVATE);
#endif
        (void)pthread_mutex_init(&m_protector, &attr);
        (void)pthread_mutexattr_destroy(&attr);
    }

    safeout::~safeout()
    {
        if(pthread_mutex_lock(&m_protector) == EOWNERDEAD)
        {
            pthread_mutex_consistent(&m_protector);
        }
        for(std::map<pthread_t, std::string>::iterator it
                                                   = m_outData.begin(); it != m_outData.end(); ++it)
        {
            *m_pos << it->second << std::endl;
        }
        (void)pthread_mutex_unlock(&m_protector);
        (void)pthread_mutex_destroy(&m_protector);
    }

    void safeout::redirect(std::ostream &os)
    {
        if(pthread_mutex_lock(&m_protector) == EOWNERDEAD)
        {
            pthread_mutex_consistent(&m_protector);
        }
        m_pos = &os;
        (void)pthread_mutex_unlock(&m_protector);
    }

    safeout& safeout::endline(safeout& so)
    {
        pthread_t thr = pthread_self();
        if(pthread_mutex_lock(&so.m_protector) == EOWNERDEAD)
        {
            pthread_mutex_consistent(&so.m_protector);
        }
        *so.m_pos << so.m_outData[thr] << std::endl;
        so.m_outData[thr] = "";
        (void)pthread_mutex_unlock(&so.m_protector);
        return so;
    }

    safeout & safeout::operator<< (safeout & (*_f)(safeout &))
    {
        return(*_f)(*this);
    }

    std::string safeout::time()
    {
        std::stringstream ss;
        struct timeval  tv;
        (void)gettimeofday(&tv, NULL);
        struct tm * ti;
        ti = localtime(&tv.tv_sec);
        ss << std::setfill('0');
        ss << std::setw(4) << ti->tm_year + 1900 << '/'
           << std::setw(2) << ti->tm_mon + 1 << '/'
           << std::setw(2) << ti->tm_mday << ' '
           << std::setw(2) << ti->tm_hour << ':'
           << std::setw(2) << ti->tm_min << ':'
           << std::setw(2) << ti->tm_sec << ':'
           << std::setw(3) << tv.tv_usec/1000;
        return ss.str();
    }

    safeout sout;

    safeout& endl(safeout& so)
    {
        return so.endline(so);
    }

    void redirect(std::ostream &os)
    {
        sout.redirect(os);
    }

}

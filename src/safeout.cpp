#include "safeout.h"
#include <iostream>
#include <sys/time.h>
#include <time.h>
#include <iomanip>

namespace dbg
{
    safeout::safeout() : m_pos(&std::cout), m_outData(), m_defMsgLevel(DBG_DEBUG)
    {
        protectorInit();
    }

    safeout::~safeout()
    {
        protectorLock();
        for(std::map<pthread_t, outData_t>::iterator it
                                                   = m_outData.begin(); it != m_outData.end(); ++it)
        {
            if(it->second.outStr !=  "")
            {
                *m_pos << it->second.outStr << std::endl;
            }
        }
        protectorUnlock();
        protectorDestroy();
    }

    void safeout::redirect(std::ostream &os)
    {
        protectorLock();
        m_pos = &os;
        protectorUnlock();
    }

    safeout& safeout::flush(safeout& so, dbgLevel_t msglev)
    {
        pthread_t thr = pthread_self();
        so.protectorLock();
        if (so.m_outData.find(thr) != so.m_outData.end())
        {
            if(so.m_outData[thr].outStr != "")
            {
                *so.m_pos << so.m_outData[thr].outStr << std::endl;
                so.m_outData[thr].outStr = "";
            }
        }
        else
        {
            so.m_outData[thr].outStr = "";
            so.m_outData[thr].dbgLev = DBG_DEBUG;
        }
        so.m_outData[thr].msgLev = msglev;
        so.protectorUnlock();
        return so;
    }

    safeout & safeout::operator<< (safeout & (*_f)(safeout &))
    {
        return(*_f)(*this);
    }

    void safeout::setDbgLevel(dbgLevel_t level)
    {
        pthread_t thr = pthread_self();
        protectorLock();
        if (m_outData.find(thr) == m_outData.end())
        {
            m_outData[thr].outStr = "";
            m_outData[thr].msgLev = m_defMsgLevel;
        }
        m_outData[thr].dbgLev = level;
        protectorUnlock();
    }

    void safeout::setDefaultMsgLevel(dbgLevel_t level)
    {
        protectorLock();
        m_defMsgLevel = level;
        protectorUnlock();
    }

    dbgLevel_t safeout::getDefaultMsgLevel()
    {
        dbgLevel_t result = DBG_NOTSET;
        protectorLock();
        result = m_defMsgLevel;
        protectorUnlock();
        return result;
    }

    dbgLevel_t safeout::getCurrentMsgLevel()
    {
        dbgLevel_t result = DBG_NOTSET;
        pthread_t thr = pthread_self();
        protectorLock();
        if (m_outData.find(thr) != m_outData.end())
        {
            result = m_outData[thr].msgLev;
        }
        protectorUnlock();
        return result;
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

    void safeout::protectorInit()
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

    void safeout::protectorDestroy()
    {
        (void)pthread_mutex_destroy(&m_protector);
    }

    void safeout::protectorLock()
    {
        if(pthread_mutex_lock(&m_protector) == EOWNERDEAD)
        {
            pthread_mutex_consistent(&m_protector);
        }
    }

    void safeout::protectorUnlock()
    {
        (void)pthread_mutex_unlock(&m_protector);
    }

    safeout sout;

    void redirect(std::ostream &os)
    {
        sout.redirect(os);
    }

    void dbgLevel(dbgLevel_t dbgLevel)
    {
        sout.setDbgLevel(dbgLevel);
    }

    void defaultMsgLevel(dbgLevel_t msgLevel)
    {
        sout.setDefaultMsgLevel(msgLevel);
    }

    safeout& endl(safeout& so)
    {
        return so.flush(so, so.getDefaultMsgLevel());
    }

    safeout& info(safeout& so)
    {
        if(so.getCurrentMsgLevel() != DBG_INFO)
        {
            (void)so.flush(so, DBG_INFO);
        }
        return so;
    }

    safeout& dbg(safeout& so)
    {
        if(so.getCurrentMsgLevel() != DBG_DEBUG)
        {
            (void)so.flush(so, DBG_DEBUG);
        }
        return so;
    }

    safeout& warn(safeout& so)
    {
        if(so.getCurrentMsgLevel() != DBG_WARNING)
        {
            (void)so.flush(so, DBG_WARNING);
        }
        return so;
    }

    safeout& err(safeout& so)
    {
        if(so.getCurrentMsgLevel() != DBG_ERROR)
        {
            (void)so.flush(so, DBG_ERROR);
        }
        return so;
    }

}

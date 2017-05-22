#ifndef SAFEOUT_H_
#define SAFEOUT_H_

#include <map>
#include <sys/types.h>
#include <string>
#include <pthread.h>
#include <sstream>
#include <ostream>
#include <errno.h>


//#define DBG_LOG_MULTUPROCESS



#define __DBG_LOG_S_(x) #x
#define __DBG_LOG_S__(x) __DBG_LOG_S_(x)
#define __S__LINE__ __DBG_LOG_S__(__LINE__)

#define __DBG_LOCATION__  (std::string(__FUNCTION__) + " " + std::string(__FILE__)\
                           + ":L" + std::string(__S__LINE__))

namespace dbg
{
    typedef enum dbgLevel
    {
        DBG_INFO,
        DBG_DEBUG,
        DBG_WARNING,
        DBG_ERROR,
        DBG_NOTSET
    }dbgLevel_t;

    class safeout
    {
    public:
        safeout();
        ~safeout();
        void redirect(std::ostream &os);
        template<class T>
        friend safeout& operator <<(safeout& so, T t)
        {
            std::stringstream ss;
            ss << t;
            pthread_t thr = pthread_self();
            so.protectorLock();
            if (so.m_outData.find(thr) == so.m_outData.end())
            {
                so.m_outData[thr].outStr = "";
                so.m_outData[thr].msgLev = so.m_defMsgLevel;
                so.m_outData[thr].dbgLev = DBG_DEBUG;
            }
            if(so.m_outData[thr].msgLev >= so.m_outData[thr].dbgLev)
            {
                if(so.m_outData[thr].outStr == "" && ss.str() != "")
                {
                    so.m_outData[thr].outStr = safeout::time();
                    switch(so.m_outData[thr].msgLev)
                    {
                    case DBG_INFO:
                        so.m_outData[thr].outStr += " [INFO ] [";
                        break;
                    case DBG_DEBUG:
                        so.m_outData[thr].outStr += " [DEBUG] [";
                        break;
                    case DBG_WARNING:
                        so.m_outData[thr].outStr += " [WARN ] [";
                        break;
                    case DBG_ERROR:
                        so.m_outData[thr].outStr += " [ERROR] [";
                        break;
                    default:
                        so.m_outData[thr].outStr += " [???  ] [";
                        break;
                    }
                    char name[32] = {0};
                    if(pthread_getname_np(thr, name, sizeof(name)))
                    {
                        name[0] = '\0';
                    }
                    so.m_outData[thr].outStr += std::string(name) + "] : ";
                }
                so.m_outData[thr].outStr += ss.str();
            }
            so.protectorUnlock();
            return so;
        }
        static safeout& flush(safeout& so, dbgLevel_t msglev);
        safeout & operator<< (safeout & (*_f)(safeout &));
        void setDbgLevel(dbgLevel_t level);
        void setDefaultMsgLevel(dbgLevel_t level);
        dbgLevel_t getDefaultMsgLevel();
        dbgLevel_t getCurrentMsgLevel();

    private:
        typedef struct outData
        {
            dbgLevel_t  msgLev;
            dbgLevel_t  dbgLev;
            std::string outStr;
        }outData_t;
        std::ostream                   * m_pos;
        std::map<pthread_t, outData_t>   m_outData;
        dbgLevel_t                       m_defMsgLevel;
        pthread_mutex_t                  m_protector;

        static std::string time();
        void protectorInit();
        void protectorDestroy();
        void protectorLock();
        void protectorUnlock();
    };

    extern safeout sout;
    void redirect(std::ostream &os);
    void dbgLevel(dbgLevel_t dbgLevel);
    void defaultMsgLevel(dbgLevel_t msgLevel);
    safeout& endl(safeout& so);
    safeout& info(safeout& so);
    safeout& dbg(safeout& so);
    safeout& warn(safeout& so);
    safeout& err(safeout& so);

}

#endif

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

namespace dbg
{
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
            if(pthread_mutex_lock(&so.m_protector) == EOWNERDEAD)
            {
                pthread_mutex_consistent(&so.m_protector);
            }
            if(so.m_outData[thr] == "")
            {
                so.m_outData[thr] = safeout::time() + " : ";
            }
            so.m_outData[thr] += ss.str();
            (void)pthread_mutex_unlock(&so.m_protector);
            return so;
        }
        static safeout& endline(safeout& so);
        safeout & operator<< (safeout & (*_f)(safeout &));

    private:
        std::ostream                   * m_pos;
        std::map<pthread_t, std::string> m_outData;
        pthread_mutex_t                  m_protector;

        static std::string time();
    };

    extern safeout sout;
    safeout& endl(safeout& so);
    void redirect(std::ostream &os);

}

#endif

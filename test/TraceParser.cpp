#include "TraceParser.h"
#include <ctype.h>

TraceParser::TraceParser(std::istream &stream) : m_is(stream),
                                                 m_date(),
                                                 m_time(),
                                                 m_level(),
                                                 m_thread(""),
                                                 m_message("")
{
}

bool TraceParser::process()
{
    bool result = false;
    enum EState{
        EState_DATE,
        EState_TIME,
        EState_LEVEL,
        EState_THREAD,
        EState_MESSAGE,
        EState_FINISH
    }state = EState_DATE;

    while(state < EState_FINISH && m_is.rdbuf()->in_avail() != 0)
    {
        std::string tmp;
        if(state != EState_MESSAGE)
        {
            m_is >> tmp;
        }
        else
        {
            std::getline(m_is, tmp);
        }
        switch(state)
        {
        case EState_DATE:
            if(isDate(tmp))
            {
                m_date = tmp;
                state = EState_TIME;
            }
            else
            {
                std::getline(m_is, tmp);
                state = EState_FINISH;
            }
            break;
        case EState_TIME:
            if(isTime(tmp))
            {
                m_time = tmp;
                state = EState_LEVEL;
            }
            else
            {
                std::getline(m_is, tmp);
                state = EState_FINISH;
            }
            break;
        case EState_LEVEL:
            if(isLevel(tmp))
            {
                m_level = tmp;
                state = EState_THREAD;
            }
            else
            {
                std::getline(m_is, tmp);
                state = EState_FINISH;
            }
            break;
        case EState_THREAD:
            if(isThread(tmp))
            {
                m_thread = tmp.substr(1, tmp.length() - 2);
                state = EState_MESSAGE;
            }
            else
            {
                std::getline(m_is, tmp);
                state = EState_FINISH;
            }
            break;
        case EState_MESSAGE:
            if(tmp.length() > 0)
            {
                m_message = tmp.erase(0, 1);
                result = true;
            }
            state = EState_FINISH;
            break;
        default:
            state = EState_FINISH;
            break;
        }
    }
    return result;
}

bool TraceParser::isDate(std::string date)
{
    bool result = false;
    if(date.length() == 10)
    {
        if(isdigit(date[0]) &&
           isdigit(date[1]) &&
           isdigit(date[2]) &&
           isdigit(date[3]) &&
           date[4] == '/'   &&
           isdigit(date[5]) &&
           isdigit(date[6]) &&
           date[7] == '/'   &&
           isdigit(date[8]) &&
           isdigit(date[9]))
        {
            result = true;
        }
    }
    return result;
}

bool TraceParser::isTime(std::string time)
{
    bool result = false;
    if(time.length() == 12)
    {
        if(isdigit(time[0]) &&
           isdigit(time[1]) &&
           time[2] == ':'   &&
           isdigit(time[3]) &&
           isdigit(time[4]) &&
           time[5] == ':'   &&
           isdigit(time[6]) &&
           isdigit(time[7]) &&
           time[8] == ':'   &&
           isdigit(time[9]) &&
           isdigit(time[10])&&
           isdigit(time[11]))
        {
            result = true;
        }
    }
    return result;
}

bool TraceParser::isLevel(std::string level)
{
    bool result = false;
    if(level == "[INFO_]" ||
       level == "[DEBUG]" ||
       level == "[WARN_]" ||
       level == "[ERROR]" ||
       level == "[???__]")
    {
        result = true;
    }
    return result;
}

bool TraceParser::isThread(std::string thread)
{
    bool result = false;
    if(thread.length() >= 2)
    {
        if(thread[0] == '[' && thread[thread.length() - 1] == ']')
        {
            result = true;
        }
    }
    return result;
}

#ifndef TRACEPARSER_H_
#define TRACEPARSER_H_

#include <istream>
#include <string>

class TraceParser
{
public:
    TraceParser(std::istream &stream);
    bool process();
    std::string date() const {return m_date;}
    std::string time() const {return m_time;}
    std::string level() const {return m_level;}
    std::string thread() const {return m_thread;}
    std::string message() const {return m_message;}

private:
    std::istream    & m_is;
    std::string       m_date;
    std::string       m_time;
    std::string       m_level;
    std::string       m_thread;
    std::string       m_message;

    static bool isDate(std::string date);
    static bool isTime(std::string time);
    static bool isLevel(std::string level);
    static bool isThread(std::string thread);

};

#endif

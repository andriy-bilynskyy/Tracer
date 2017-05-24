#include "gtest/gtest.h"
#include "safeout.h"
#include <sstream>
#include "TraceParser.h"
#include <sys/time.h>
#include <time.h>
#include <pthread.h>


class CDBGtest : public ::testing::Test {
public:
    CDBGtest() : m_ss()
    {
        dbg::sout.redirect(m_ss);
    }

protected:
    std::stringstream m_ss;

    static void * theadFunc(void *)
    {
        pthread_setname_np(pthread_self(), "worker");

        dbg::sout << "worker" << dbg::endl;

        return NULL;
    }
};

TEST_F(CDBGtest, Simple)
{
    ASSERT_EQ(pthread_setname_np(pthread_self(), "gtest"), 0);

    dbg::sout << "test" << dbg::endl;

    TraceParser tp(m_ss);
    ASSERT_TRUE(tp.process());
    ASSERT_STRCASEEQ(tp.level().c_str(), "[DEBUG]");
    ASSERT_STRCASEEQ(tp.thread().c_str(), "gtest");
    ASSERT_STRCASEEQ(tp.message().c_str(), "test");
    ASSERT_FALSE(tp.process());
}

TEST_F(CDBGtest, Timestamp)
{
    dbg::sout << "test" << dbg::endl;

    struct timeval  tv;
    (void)gettimeofday(&tv, NULL);
    std::stringstream timeStamp;
    struct tm * ti;
    ti = localtime(&tv.tv_sec);
    timeStamp << std::setfill('0');
    TraceParser tp(m_ss);
    ASSERT_TRUE(tp.process());
    timeStamp << std::setw(4) << ti->tm_year + 1900 << '/'
              << std::setw(2) << ti->tm_mon + 1 << '/'
              << std::setw(2) << ti->tm_mday << ' '
              << std::setw(2) << ti->tm_hour << ':'
              << std::setw(2) << ti->tm_min << ':'
              << std::setw(2) << ti->tm_sec << ':'
              << std::setw(3) << tv.tv_usec/1000;
    std::string retTimestamp = tp.date() + ' ' + tp.time();
    ASSERT_STRCASEEQ(retTimestamp.substr(0, retTimestamp.length() -1).c_str(),
                     timeStamp.str().substr(0, timeStamp.str().length() -1).c_str());
    ASSERT_FALSE(tp.process());
}

TEST_F(CDBGtest, Multithread)
{
    ASSERT_EQ(pthread_setname_np(pthread_self(), "main"), 0);
    pthread_t worker;
    ASSERT_EQ(pthread_create(&worker, NULL, theadFunc, NULL), 0);

    dbg::sout << "main" << dbg::endl;

    ASSERT_EQ(pthread_join(worker, NULL), 0);
    TraceParser tp(m_ss);
    ASSERT_TRUE(tp.process());
    ASSERT_STRCASEEQ(tp.thread().c_str(), tp.message().c_str());
    ASSERT_TRUE(tp.message() == "main" || tp.message() == "worker");
    ASSERT_TRUE(tp.process());
    ASSERT_STRCASEEQ(tp.thread().c_str(), tp.message().c_str());
    ASSERT_TRUE(tp.message() == "main" || tp.message() == "worker");
    ASSERT_FALSE(tp.process());
}

TEST_F(CDBGtest, DebugLevel)
{
    TraceParser tp(m_ss);

    dbg::sout << "debug"
              << dbg::info << "info"
              << dbg::warn << "warn"
              << dbg::err << "error" << dbg::endl;

    ASSERT_TRUE(tp.process());
    ASSERT_TRUE(tp.process());
    ASSERT_TRUE(tp.process());
    ASSERT_FALSE(tp.process());

    dbg::sout.setDbgLevel(dbg::DBG_INFO);
    dbg::sout << "debug"
              << dbg::info << "info"
              << dbg::warn << "warn"
              << dbg::err << "error" << dbg::endl;

    ASSERT_TRUE(tp.process());
    ASSERT_TRUE(tp.process());
    ASSERT_TRUE(tp.process());
    ASSERT_TRUE(tp.process());
    ASSERT_FALSE(tp.process());

    dbg::sout.setDbgLevel(dbg::DBG_WARNING);
    dbg::sout << "debug"
              << dbg::info << "info"
              << dbg::warn << "warn"
              << dbg::err << "error" << dbg::endl;

    ASSERT_TRUE(tp.process());
    ASSERT_TRUE(tp.process());
    ASSERT_FALSE(tp.process());

    dbg::sout.setDbgLevel(dbg::DBG_ERROR);
    dbg::sout << "debug"
              << dbg::info << "info"
              << dbg::warn << "warn"
              << dbg::err << "error" << dbg::endl;

    ASSERT_TRUE(tp.process());
    ASSERT_FALSE(tp.process());

    dbg::sout.setDbgLevel(dbg::DBG_NOTSET);
    dbg::sout << "debug"
              << dbg::info << "info"
              << dbg::warn << "warn"
              << dbg::err << "error" << dbg::endl;

    ASSERT_FALSE(tp.process());

    dbg::sout.setDbgLevel(dbg::DBG_DEBUG);
    dbg::sout << "debug"
              << dbg::info << "info"
              << dbg::warn << "warn"
              << dbg::err << "error" << dbg::endl;

    ASSERT_TRUE(tp.process());
    ASSERT_TRUE(tp.process());
    ASSERT_TRUE(tp.process());
    ASSERT_FALSE(tp.process());
}

TEST_F(CDBGtest, DefaultMsgLevel)
{
    TraceParser tp(m_ss);

    dbg::sout.setDbgLevel(dbg::DBG_INFO);

    dbg::sout.setDefaultMsgLevel(dbg::DBG_INFO);

    ASSERT_EQ(dbg::sout.getDefaultMsgLevel(), dbg::DBG_INFO);

    dbg::sout << "message" << dbg::endl;

    ASSERT_EQ(dbg::sout.getCurrentMsgLevel(), dbg::DBG_INFO);
    ASSERT_TRUE(tp.process());
    ASSERT_STRCASEEQ(tp.level().c_str(), "[INFO_]");

    dbg::sout.setDefaultMsgLevel(dbg::DBG_DEBUG);

    ASSERT_EQ(dbg::sout.getDefaultMsgLevel(), dbg::DBG_DEBUG);

    dbg::sout << "message" << dbg::endl;

    ASSERT_EQ(dbg::sout.getCurrentMsgLevel(), dbg::DBG_DEBUG);
    ASSERT_TRUE(tp.process());
    ASSERT_STRCASEEQ(tp.level().c_str(), "[DEBUG]");

    dbg::sout.setDefaultMsgLevel(dbg::DBG_WARNING);

    ASSERT_EQ(dbg::sout.getDefaultMsgLevel(), dbg::DBG_WARNING);

    dbg::sout << "message" << dbg::endl;

    ASSERT_EQ(dbg::sout.getCurrentMsgLevel(), dbg::DBG_WARNING);
    ASSERT_TRUE(tp.process());
    ASSERT_STRCASEEQ(tp.level().c_str(), "[WARN_]");

    dbg::sout.setDefaultMsgLevel(dbg::DBG_ERROR);

    ASSERT_EQ(dbg::sout.getDefaultMsgLevel(), dbg::DBG_ERROR);

    dbg::sout << "message" << dbg::endl;

    ASSERT_EQ(dbg::sout.getCurrentMsgLevel(), dbg::DBG_ERROR);
    ASSERT_TRUE(tp.process());
    ASSERT_STRCASEEQ(tp.level().c_str(), "[ERROR]");

    dbg::sout.setDbgLevel(dbg::DBG_DEBUG);
    dbg::sout.setDefaultMsgLevel(dbg::DBG_DEBUG);
    ASSERT_FALSE(tp.process());
}

TEST_F(CDBGtest, NOendl)
{
    TraceParser tp(m_ss);

    dbg::sout << "message";

    ASSERT_FALSE(tp.process());

    dbg::sout << dbg::endl;

    ASSERT_TRUE(tp.process());
    ASSERT_STRCASEEQ(tp.message().c_str(), "message");
    ASSERT_FALSE(tp.process());

    dbg::sout << dbg::endl;

    ASSERT_FALSE(tp.process());

    dbg::sout << "message";

    ASSERT_FALSE(tp.process());

    dbg::sout.setDefaultMsgLevel(dbg::DBG_ERROR);

    ASSERT_TRUE(tp.process());
    ASSERT_STRCASEEQ(tp.message().c_str(), "message");
    ASSERT_FALSE(tp.process());

}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}





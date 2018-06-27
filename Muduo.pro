TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    Base/CountDownLatch.cpp \
    Base/Condition.cpp \
    Base/Date.cpp \
    Base/Exception.cpp \
    Base/Thread.cpp \
    Base/Timestamp.cpp \
    Base/LogStream.cpp \
    Base/Logging.cpp \
    Base/TimeZone.cpp \
    Base/FileUtil.cpp \
    Base/ProcessInfo.cpp \
    Base/ThreadPool.cpp \
    Tests/threadtest.cpp

include(deployment.pri)
qtcAddDeployment()

LIBS +=-lpthread

#INCLUDEPATH += F:/boost_1_56/bin/include
#INCLUDEPATH +=D:/boost_1_67_0

#LIBS +=F:/boost_1_56/bin/lib/

HEADERS += \
    Base/Copyable.h \
    Base/StringPiece.h \
    Base/Types.h \
    Base/Atomic.h \
    Base/BlockingQueue.h \
    Base/BoundedBlockingQueue.h \
    Base/Condition.h \
    Base/CurrentThread.h \
    Base/CountDownLatch.h \
    Base/Date.h \
    Base/Exception.h \
    Base/Thread.h \
    Base/Timestamp.h \
    Base/LogStream.h \
    Base/Logging.h \
    Base/TimeZone.h \
    Base/FileUtil.h \
    Base/Mutex.h \
    Base/NonCopyable.h \
    Base/Singleton.h \
    Base/ThreadLocal.h \
    Base/WeakCallback.h \
    Base/ProcessInfo.h \
    Base/GzipFile.h \
    Base/GzipFile.h \
    Base/ThreadLocalSingleton.h \
    Base/ThreadLocalSingleton.h \
    Base/ThreadPool.h

DISTFILES +=


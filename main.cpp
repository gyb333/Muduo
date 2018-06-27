#include <iostream>
#include <Base/StringPiece.h>
#include <Base/Atomic.h>
#include <Base/Thread.h>
#include <Base/BlockingQueue.h>
#include <Base/BoundedBlockingQueue.h>
#include <Base/FileUtil.h>
#include <Base/Singleton.h>
#include <Base/ThreadPool.h>
#include <Base/ThreadLocalSingleton.h>

#include <Tests/threadtest.cpp>

using namespace std;

int main()
{
    cout << "Hello World!" << endl;
    muduo::ThreadPool pool("MainThreadPool");
    pool.setMaxQueueSize(10);
    pool.start(5);
    //Test_Thread();
    return 0;
}


#include <Base/CountDownLatch.h>
#include <Base/Mutex.h>
#include <Base/Thread.h>
#include <Base/Timestamp.h>

#include <boost/bind.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <vector>
#include <stdio.h>
#include <iostream>

using namespace  muduo;
using namespace std;

MutexLock g_mutex;
vector<int> g_vec;
const int kCount = 10*1000*1000;

void threadFunc()
{
    cout<<"before "<<endl;
    MutexLockGuard lock(g_mutex);
    cout<<"after 1"<<endl;
    g_vec.clear();
    for (int i = 0; i < kCount; ++i)
      {
        g_vec.push_back(i);

      }
    cout<<"after 2"<<endl;
}





void Test_Thread()
{

    const int kMaxThreads = 8;
    g_vec.reserve(kMaxThreads * kCount);
    boost::ptr_vector<Thread> threads;

    for (int nthreads = 1; nthreads < kMaxThreads; ++nthreads)
    {
        threads.push_back(new Thread(&threadFunc));
        threads.back().start();

    }

    for(Thread& each : threads){
        each.join();
    }

}

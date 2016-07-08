/* 
 * An experimental implementation of a Thread Pool
 * using C++11 features.
 *
 * Notes, common producer/consumer threading code pattern:
 *   lock
 *   While loop
 *      wait on condition variable when condition not met
 *       (lock will be dropped automatically when waiting)
 *   Wake up (automatically get the lock)
 *   	<do something.>
 *   condition variable notify
 *   unlock
 */

#include <thread>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <vector>

using namespace std;

class ThreadPool;

typedef enum 
{
	NOT_STARTED,
	STARTING,
	STARTED,
	STOPPING,
	STOPPED
} thread_pool_state;

typedef enum
{
	PS_OK = 0,
	PS_FAIL = 1,
	PS_RETRY =2
	
} post_status;


// User can derive with more complicated work.
class Work
{
public:
	Work();
	virtual ~Work();
	virtual void doWork();
};


class MyThread 
{
public:
	MyThread(ThreadPool* p);
	~MyThread();
		
	void run();
private:
	ThreadPool* _pool;
};

class ThreadPool
{
public:
	ThreadPool (unsigned int t_size, // how many threads.
		    unsigned int w_size);
		    // max of how many waiting work items.
	~ThreadPool ();

	void start();
	void stop();

	post_status postWork(Work& w);

private:
	// work queue
	deque<Work> _works;

	// mutex
	std::mutex _m;

	// condition variable
	std::condition_variable _cv;

	// a vector of threads
	vector<std::thread> _threads;

	unsigned int _t_size;
	unsigned int _w_size;

	thread_pool_state _state;

	friend class MyThread;
};


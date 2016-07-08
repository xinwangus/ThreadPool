#include "ThreadPool.h"
#include <iostream>

// Work methods
Work::
Work()
{
}

Work::
~Work()
{
}

void Work::
doWork()
{
}


// MyThread Methods
MyThread::
MyThread(ThreadPool* p)
:_pool(p)
{
}

MyThread::
~MyThread()
{
}

void
MyThread::
run()
{
	if (_pool == 0) { return; }
	Work* mywork; 

	while (1) {
	    {
		std::unique_lock<std::mutex> uqlk(_pool->_m);

		while (_pool->_works.empty()) {
			// This unlocks _m.
			_pool->_cv.wait(uqlk);
		}
		
		// Bail out.
		if ((_pool->_state == STOPPED) ||
		    ((_pool->_state == STOPPING) && 
		     (_pool->_works.empty()))) {
			return;
		}

		// we have the lock _m again.
		if (!_pool->_works.empty()) {
			mywork = &(_pool->_works.front());
			_pool->_works.pop_front();
		}
		// tell the other thread.
		if (!_pool->_works.empty()) {
			_pool->_cv.notify_one();
		}
	    }
	    mywork->doWork();
	    delete mywork;
	}
}


// ThreadPool Methods
ThreadPool::
ThreadPool(unsigned int t_size, 
           unsigned int w_size)
:_t_size(t_size),
 _w_size(t_size + w_size),
 _state(NOT_STARTED)
{
}

ThreadPool::
~ThreadPool()
{ // TODO
} 

void
my_test(MyThread* t)
{
	t->run();
}

void
ThreadPool::
start()
{
	{
                std::unique_lock<std::mutex> uqlk(_m);
		_state = STARTING;
	}
	
	for (int i = 0; i < _t_size; i++) {
		MyThread* mt = new MyThread(this);
		// ??? Does not compile???
		std::thread t(my_test, std::ref(mt));
		_threads.push_back(t);
	}

	{
                std::unique_lock<std::mutex> uqlk(_m);
		_state = STARTED;
	}
}


post_status
ThreadPool::
postWork(Work& w)
{
        {
                std::unique_lock<std::mutex> uqlk(_m);

		switch (_state) 
		{
			case NOT_STARTED:
			case STARTING:
				return PS_RETRY;
				break;

			case STOPPING:
        		case STOPPED:
				return PS_FAIL;
				break;

        		case STARTED:
			default:
				break;
		}

		if (_works.size() > _w_size) {
			return PS_RETRY;
		}

                _works.push_back(w);

                // tell the other thread.
                _cv.notify_one();
        }
	return PS_OK;
}

class MyWork : public Work
{
public:
	void doWork() {
		cout << "This is just a test." << endl;
	}	
};


int
main()
{
	ThreadPool pool(4,8);
	pool.start();

	MyWork* w = new MyWork();
	pool.postWork(*w);

	return 0;
}


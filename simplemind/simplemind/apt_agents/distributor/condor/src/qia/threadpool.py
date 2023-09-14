import sys
import multiprocessing
from queue import Queue, Empty, Full
from threading import Thread, Event, Lock
import time
import traceback
from collections import deque
from simplemind.apt_agents.distributor.condor.src.qia.exceptions import TimedOut, ErrorValue

def _worker(queue, error_callback, is_closed, item_consumed):
    while True:
        try:
            if is_closed() and queue.empty():
                time.sleep(0.5) #Just in case some other threads are still adding jobs to the queue
                if queue.empty():
                    break
            item = queue.get()
            if item[0](*item[1], **item[2]):
                item_consumed()
            else:
                queue.put(item)
            queue.task_done()
        except Empty:
            pass
        except:
            error_callback(sys.exc_info())
            queue.task_done()
    
class _AsynResult:
    def __init__(self):
        self._value = None
        self._event = Event()
        self._event.clear()
        
    def set(self, val):
        self._value = val
        self._event.set()
        
    def get(self, timeout=None):
        if self._event.wait(timeout):
            return self._value
        else:
            raise TimedOut

def _async_wrapper(async_result, func, args, kwargs):
    try:
        output = func(*args, **kwargs)
        async_result.set(output)
    except:
        async_result.set(ErrorValue(sys.exc_info(), traceback.format_exc()))
    return True
    
def _queue_join(queue, all_done):
    queue.join()
    all_done.set()

def _asynresult_iter(results):
    for i in results:
        value = i.get()
        if isinstance(value, ErrorValue):
            # value.raise_exception()
            print("This is an:", ErrorValue)
            print(value)
        
        yield value
    
class _AsynResultContainer:
    def __init__(self, results):
        self.__results = results
    
    def __iter__(self):
        return _asynresult_iter(self.__results)
        
    def values(self):
        return self.__results
           
class ThreadPool:
    def __init__(self, parallelnum=None, poolsize=None, error_callback=None):
        if parallelnum is None:
            parallelnum = multiprocessing.cpu_count()-1
        if poolsize is None:
            poolsize = parallelnum

        print("THREADPOOL parallelnum: ", parallelnum)    #potentially can increase how many condor jobs added
        print("THREADPOOL SIZE: ", poolsize)    #potentially can increase how many condor jobs added
        self._error_callback = error_callback
        self._pool = Queue(maxsize=poolsize)
        self._all_done = Event()
        self._all_done.clear()
        self._print_lock = Lock()
        self._is_closed = False
        
        self._put_lock = Lock()
        self._pool_free = Event()
        self._pool_free.set()
        
        self._pool_job_num = 0
        self._count_lock = Lock()
        
        self._workers = [Thread(target=_worker, args=[self._pool, self._actual_error_callback, self.is_closed, self._item_consumed]) for i in range(parallelnum)]
        for w in self._workers:
            w.daemon = True
            w.start()
        print("Number of workers:", len(self._workers))

        self._pool_join_thread = Thread(target=_queue_join, args=[self._pool, self._all_done])
        
    def _item_consumed(self):
        self._pool_free.set()

    def __del__(self):
        self.close()
        
    def _check_pool(self, timeout):
        if not self._put_lock.acquire(timeout=-1 if timeout is None else timeout):
            raise Full()
        if self._pool.maxsize>0:
            if self._pool.unfinished_tasks>=self._pool.maxsize:
                self._pool_free.clear()
        if not self._pool_free.wait(timeout=timeout):
            raise Full()
            
    def is_closed(self):
        return self._is_closed
            
    def close(self):
        if self._is_closed:
            return
        self._is_closed = True
        self._pool.join()
        self._workers = None
        self._pool_join_thread = None

    def put(self, func, args=None, kwargs=None, timeout=None):
        if self.is_closed():
            raise SystemError("Pool is already closed!")
        self._check_pool(timeout)
        if args is None:
            args = []
        if kwargs is None:
            kwargs = {}
        self._pool.put((func, args, kwargs), timeout=timeout)
        self._put_lock.release()
        
    def apply(self, func, args=None, kwargs=None, timeout=None):
        if self.is_closed():
            raise SystemError("Pool is already closed!")
        self._check_pool(timeout)
        if args is None:
            args = []
        if kwargs is None:
            kwargs = {}
        ret = _AsynResult()
        self._pool.put((_async_wrapper, (ret, func, args, kwargs), {}), timeout=timeout)
        self._put_lock.release()
        return ret
        
    def map(self, func, iter):
        result = self.async_map(func, iter)
        return _AsynResultContainer(result)
        
    def async_map(self, func, iter):
        if self.is_closed():
            raise SystemError("Pool is already closed!")
        result = []
        for i in iter:
            result.append(self.apply(func, args=(i,)))
        return result
        
    def join(self, timeout=None):
        if self.is_closed():
            raise SystemError("Pool is already closed!")
        if not self._pool_join_thread.is_alive():
            self._all_done.clear()
            self._pool_join_thread.start()    #MWW 10302019
        return self._all_done.wait(timeout=timeout)
        
    def _actual_error_callback(self, exc_info):
        if self._error_callback is None:
            self._print_lock.acquire()
            for i in traceback.format_exception_only(exc_info[0],exc_info[1]):
                print(i.strip())
            self._print_lock.release()
            return
        self._error_callback(exc_info)
        
    def get_print_lock(self):
        return self._print_lock
        
    def __enter__(self):
        return self
        
    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()
        return False

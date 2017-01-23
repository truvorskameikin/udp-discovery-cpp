#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#if defined(__APPLE__) || defined(__ANDROID__) || defined(__gnu_linux__)
#define HAS_PTHREAD
#define HAS_POSIX_SLEEP
#endif

#if defined(HAS_PTHREAD)
#include <stdlib.h>
#include <pthread.h>
#endif

#if defined(HAS_POSIX_SLEEP)
#include <unistd.h>
#endif

#include "thread_routines.h"

#include <iostream>

namespace threadroutines {
  namespace impl {
#if defined(_WIN32)
    class ThreadWin32 : public ThreadImplInterface {
     public:
      ThreadWin32() : thread_handle_(0) {
      }

      void Start(ThreadWorker* worker) {
        if (thread_handle_) {
          delete worker;
          return;
        }

        thread_handle_ = CreateThread(NULL, 0, threadFunc, worker, 0, NULL);
      }

      void Detach() {
        if (thread_handle_)
          CloseHandle(thread_handle_);
        thread_handle_ = 0;
      }

      void Join() {
        if (thread_handle_) {
          WaitForSingleObject(thread_handle_, INFINITE);
          CloseHandle(thread_handle_);
        }

        thread_handle_ = 0;
      }

     private:
      static
      DWORD WINAPI threadFunc(void* parameters) {
        ThreadWorker* worker = reinterpret_cast<ThreadWorker*>(parameters);
        smartptrroutines::shared_ptr<ThreadWorker> worker_wrapped(worker);

        worker_wrapped->DoWork();

        return 0;
      }

      HANDLE thread_handle_;
    };
#endif

#if defined(HAS_PTHREAD)
    class ThreadPThread : public ThreadImplInterface {
     public:
      ThreadPThread() : is_started_(false) {
      }

      void Start(ThreadWorker* worker) {
        if (is_started_) {
          delete worker;
          return;
        }

        pthread_create(&thread_, 0, threadFunc, worker);
        is_started_ = true;
      }

      void Detach() {
        if (is_started_)
          pthread_detach(thread_);
        is_started_ = false;
      }

      void Join() {
        if (is_started_) {
          void* result = 0;
          pthread_join(thread_, &result);
        }

        is_started_ = false;
      }

     private:
      static
      void* threadFunc(void* parameters) {
        ThreadWorker* worker = reinterpret_cast<ThreadWorker*>(parameters);
        smartptrroutines::shared_ptr<ThreadWorker> worker_wrapped(worker);

        worker_wrapped->DoWork();

        return 0;
      }

      pthread_t thread_;
      bool is_started_;
    };
#endif

#if defined(_WIN32)
    class MutexWin32 : public MutexImplInterface {
     public:
      MutexWin32() : is_locked_(false) {
        InitializeCriticalSection(&critical_section_);
      }

      ~MutexWin32() {
        DeleteCriticalSection(&critical_section_);
      }

      void Lock() {
        EnterCriticalSection(&critical_section_);

        while (is_locked_) {
          Sleep(1);
        }
        is_locked_ = true;
      }

      bool TryLock() {
        if (TryEnterCriticalSection(&critical_section_) == 0)
          return false;

        if (is_locked_)
          return false;

        is_locked_ = true;

        return true;
      }

      void Unlock() {
        LeaveCriticalSection(&critical_section_);
        is_locked_ = false;
      }

     private:
      CRITICAL_SECTION critical_section_;
      bool is_locked_;
    };
#endif

#if defined(HAS_PTHREAD)
    class MutexPThread : public MutexImplInterface {
     public:
      MutexPThread() : is_created_(false) {
        if (pthread_mutex_init(&mutex_, 0) == 0)
          is_created_ = true;
      }

      ~MutexPThread() {
        if (is_created_)
          pthread_mutex_destroy(&mutex_);
      }

      void Lock() {
        if (is_created_)
          pthread_mutex_lock(&mutex_);
      }

      bool TryLock() {
        if (!is_created_)
          return false;

        return pthread_mutex_trylock(&mutex_) == 0;
      }

      void Unlock() {
        if (is_created_)
          pthread_mutex_unlock(&mutex_);
      }

     private:
      pthread_mutex_t mutex_;
      bool is_created_;
    };
#endif
  }

  // thread
  thread::thread() : joinable_(false) {
  }

#if defined(__cpp_rvalue_references)
  thread::thread(thread&& rhv)
      : impl_(std::move(rhv.impl_)),
        joinable_(std::move(rhv.joinable_)) {
    rhv.clear();
  }
#endif

  thread::~thread() {
    detach();
  }

#if defined(__cpp_rvalue_references)
  thread& thread::operator=(thread&& rhv) {
    impl_ = std::move(rhv.impl_);
    joinable_ = std::move(rhv.joinable_);
    rhv.clear();

    return (*this);
  }
#endif

  void thread::swap(thread& rhv) {
    std::swap(impl_, rhv.impl_);
    std::swap(joinable_, rhv.joinable_);
  }

  bool thread::joinable() const {
    return joinable_;
  }

  void thread::join() {
    if (!joinable_)
      return;
    impl_->Join();
  }

  void thread::detach() {
    if (!joinable_)
      return;
    impl_->Detach();
    joinable_ = false;
  }

  void thread::create() {
    impl_.reset();
    joinable_ = false;

#if defined(_WIN32)
    impl_ = smartptrroutines::make_shared<impl::ThreadWin32>();
#endif

#if defined(HAS_PTHREAD)
    impl_ = smartptrroutines::make_shared<impl::ThreadPThread>();
#endif

    if (impl_)
      joinable_ = true;
  }

  void thread::clear() {
    joinable_ = false;
  }

  // this_thread
  namespace this_thread {
    void sleep_for(double seconds) {
#if defined(_WIN32)
      DWORD milliseconds = static_cast<DWORD>(seconds * 1000.0f);
      Sleep(milliseconds);
#endif

#if defined(HAS_POSIX_SLEEP)
      useconds_t usec = static_cast<unsigned int>(seconds * 1000000.0);
      usleep(usec);
#endif
    }

    void sleep_for(const chronoroutines::milliseconds& wait_milliseconds) {
#if defined(_WIN32)
      Sleep(wait_milliseconds.count());
#endif

#if defined(HAS_POSIX_SLEEP)
      usleep((useconds_t) wait_milliseconds.count() * 1000);
#endif
    }
  }

  // mutex
  mutex::mutex() {
#if defined(_WIN32)
    impl_ = smartptrroutines::make_shared<impl::MutexWin32>();
#endif

#if defined(HAS_PTHREAD)
    impl_ = smartptrroutines::make_shared<impl::MutexPThread>();
#endif
  }

  void mutex::lock() {
    if (impl_)
      impl_->Lock();
  }

  bool mutex::try_lock() {
    if (!impl_)
      return false;
    return impl_->TryLock();
  }

  void mutex::unlock() {
    if (impl_)
      impl_->Unlock();
  }

  // timed_mutex
  timed_mutex::timed_mutex() {
  }

  void timed_mutex::lock() {
    mutex_.lock();
  }

  bool timed_mutex::try_lock() {
    return mutex_.try_lock();
  }

  bool timed_mutex::try_lock_for(const chronoroutines::milliseconds& wait_milliseconds) {
    if (wait_milliseconds.count() > 0) {
      long sleep_time = 1;

      chronoroutines::steady_clock::time_point start_time = chronoroutines::steady_clock::now();
      while (true) {
        if (mutex_.try_lock())
          return true;

        this_thread::sleep_for(chronoroutines::milliseconds(sleep_time));

        chronoroutines::steady_clock::time_point cur_time = chronoroutines::steady_clock::now();
        chronoroutines::milliseconds already_waited = cur_time - start_time;

        if (already_waited > wait_milliseconds)
          break;

        sleep_time = sleep_time * 2;
        if (sleep_time > (wait_milliseconds - already_waited).count())
          sleep_time = (wait_milliseconds - already_waited).count();
      }
    }

    return mutex_.try_lock();
  }

  void timed_mutex::unlock() {
    mutex_.unlock();
  }
}

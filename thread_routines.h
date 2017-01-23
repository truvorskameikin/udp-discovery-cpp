#ifndef __THREAD_ROUTINES_H_
#define __THREAD_ROUTINES_H_

#include "smart_ptr_routines.h"
#include "chrono_routines.h"

namespace threadroutines {
  namespace impl {
    class ThreadWorker {
     public:
      virtual ~ThreadWorker() {
      }

      virtual void DoWork() = 0;
    };

    class ThreadImplInterface {
     public:
      virtual ~ThreadImplInterface() {
      }

      // implementation will delete worker
      virtual void Start(impl::ThreadWorker* worker) = 0;
      virtual void Detach() = 0;
      virtual void Join() = 0;
    };

    class MutexImplInterface {
     public:
      virtual ~MutexImplInterface() {
      }

      virtual void Lock() = 0;
      virtual bool TryLock() = 0;
      virtual void Unlock() = 0;
    };
  };

  // thread
  class thread {
   public:
    thread();

    template<typename F>
    thread(const F& f) {
      create();

      if (impl_) {
        impl::ThreadWorker* worker = new Worker<F>(f);
        impl_->Start(worker);
      }
    }

    template<typename F, typename A1>
    thread(const F& f, const A1& a1) {
      create();

      if (impl_) {
        impl::ThreadWorker* worker = new Worker1<F, A1>(f, a1);
        impl_->Start(worker);
      }
    }

#if defined(__cpp_rvalue_references)
    thread(thread&& rhv);
#endif

    ~thread();

#if defined(__cpp_rvalue_references)
    thread& operator=(thread&& rhv);
#endif

    void swap(thread& rhv);

    bool joinable() const;
    void join();
    void detach();

   private:
    thread(const thread&);
    thread& operator=(const thread&);

    void create();
    void clear();

    template<typename F>
    class Worker : public impl::ThreadWorker {
     public:
      Worker(const F& f) {
        f_ = f;
      }

      void DoWork() {
        f_();
      }

     private:
      F f_;
    };

    template<typename F, typename A1>
    class Worker1 : public impl::ThreadWorker {
     public:
      Worker1(const F& f, const A1& a1) : a1_(a1) {
        f_ = f;
      }

      void DoWork() {
        f_(a1_);
      }

     private:
      F f_;
      A1 a1_;
    };

   private:
    smartptrroutines::shared_ptr<impl::ThreadImplInterface> impl_;
    bool joinable_;
  };

  // this_thread
  namespace this_thread {
    void sleep_for(double seconds);
    void sleep_for(const chronoroutines::milliseconds& wait_milliseconds);
  };

  // mutex
  class mutex {
   public:
    mutex();

    void lock();
    bool try_lock();
    void unlock();

   private:
    mutex(const mutex&);
    mutex& operator=(mutex&);

   private:
    smartptrroutines::shared_ptr<impl::MutexImplInterface> impl_;
  };

  class timed_mutex {
   public:
    timed_mutex();

    void lock();
    bool try_lock();
    bool try_lock_for(const chronoroutines::milliseconds& wait_milliseconds);
    void unlock();

   private:
    timed_mutex(const timed_mutex&);
    timed_mutex& operator=(timed_mutex&);

   private:
    mutex mutex_;
  };

  // lock_guard
  template<typename T>
  class lock_guard {
   public:
    typedef T mutex_type;

   public:
    lock_guard(mutex_type& mutex) : m_(mutex) {
      m_.lock();
    }

    ~lock_guard() {
      m_.unlock();
    }

   private:
    mutex_type& m_;
  };

  template<typename T>
  class promise_shared_state {
   public:
    promise_shared_state() : state_(kIdle) {
      mutex_.lock();
    }

    ~promise_shared_state() {
      mutex_.unlock();
    }

    void set_value(const T& value) {
      if (state_ == kReady)
        return;

      state_ = kReady;
      value_ = value;

      mutex_.unlock();
    }

    bool try_lock_for(const chronoroutines::milliseconds& wait_milliseconds) {
      return mutex_.try_lock_for(wait_milliseconds);
    }

    const T& value() const {
      return value_;
    }

    T& value() {
      return value_;
    }

   private:
    enum State {
      kIdle,
      kReady
    };

    timed_mutex mutex_;
    State state_;
    T value_;
  };

  namespace future_status {
    enum status {
      ready,
      timeout
    };
  };

  template<typename T>
  class future {
   public:
    future() {
    }

    future(const smartptrroutines::shared_ptr<promise_shared_state<T> >& state) : state_(state) {
    }

    future(const future& rhv) : state_(rhv.state_) {
    }

#if defined(__cpp_rvalue_references)
    future(future&& rhv) : state_(std::move(rhv.state_)) {
    }
#endif

    future& operator=(const future& rhv) {
      state_ = rhv.state_;
      return (*this);
    }

#if defined(__cpp_rvalue_references)
    future& operator=(const future&& rhv) {
      state_ = std::move(rhv.state_);
      return (*this);
    }
#endif

    future_status::status wait_for(const chronoroutines::milliseconds& wait_milliseconds) {
      if (!state_->try_lock_for(wait_milliseconds))
        return future_status::timeout;
      return future_status::ready;
    }

    const T& get() const {
      return state_->value();
    }

    T& get() {
      return state_->value();
    }

   private:
    smartptrroutines::shared_ptr<promise_shared_state<T> > state_;
  };

  template<typename T>
  class promise {
   public:
    promise() {
      state_ = smartptrroutines::make_shared<promise_shared_state<T> >();
    }

    promise(const promise& rhv) : state_(rhv.state_) {
    }

#if defined(__cpp_rvalue_references)
    promise(promise&& rhv) : state_(std::move(rhv.state_)) {
    }
#endif

    promise& operator=(const promise& rhv) {
      state_ = rhv.state_;
      return (*this);
    }

#if defined(__cpp_rvalue_references)
    promise& operator=(promise&& rhv) {
      state_ = std::move(rhv.state_);
      return (*this);
    }
#endif

    ~promise() {
    }

    future<T> get_future() const {
      return future<T>(state_);
    }

    void set_value(const T& t) {
      state_->set_value(t);
    }

   private:
    smartptrroutines::shared_ptr<promise_shared_state<T> > state_;
  };
}

namespace std {
  inline
  void swap(threadroutines::thread& lhv, threadroutines::thread& rhv) {
    lhv.swap(rhv);
  }
};

#endif // #ifndef __THREAD_ROUTINES_H_

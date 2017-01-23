#ifndef __CHRONO_ROUTINES_H_
#define __CHRONO_ROUTINES_H_

namespace chronoroutines {
  template<long n, long d = 1>
  class ratio {
   public:
    static long num() {
      return n;
    }

    static long den() {
      return d;
    }
  };

  template<typename T, typename R = ratio<1> >
  class duration {
   public:
    typedef T rep;
    typedef R period;

   public:
    duration() : ticks_(0) {
    }

    duration(const T& ticks) : ticks_(ticks) {
    }

    duration(const duration& rhv) : ticks_(rhv.ticks_) {
    }

    duration& operator=(const duration& rhv) {
      ticks_ = rhv.ticks_;
      return (*this);
    }

    duration operator+(const duration& rhv) const {
      return duration(ticks_ + rhv.ticks_);
    }

    duration operator-(const duration& rhv) const {
      return duration(ticks_ - rhv.ticks_);
    }

    duration& operator+=(const duration& rhv) {
      ticks_ += rhv.ticks_;
      return (*this);
    }

    duration& operator-=(const duration& rhv) {
      ticks_ -= rhv.ticks_;
      return (*this);
    }

    duration operator+() const {
      return duration(ticks_);
    }

    duration operator-() const {
      return duration(-ticks_);
    }

    bool operator<(const duration& rhv) const {
      return ticks_ < rhv.ticks_;
    }

    bool operator>(const duration& rhv) const {
      return ticks_ > rhv.ticks_;
    }

    T count() const {
      return ticks_;
    }

   private:
    T ticks_;
  };

  typedef duration<long, ratio<60, 1> > minutes;
  typedef duration<long, ratio<1, 1> > seconds;
  typedef duration<long, ratio<1, 1000> > milliseconds;

  template<typename To, typename T, typename R>
  To duration_cast(const duration<T, R>& d) {
    return (To(d.count() * (R::num() * To::period::den()) / (To::period::num() * R::den())));
  }

  template<typename C, typename D>
  class time_point_t {
   public:
    time_point_t() {
    }

    time_point_t(const D& d) : d_(d) {
    }

    D operator-(const time_point_t& rhv) {
      return D(d_.count() - rhv.d_.count());
    }

    bool operator<(const time_point_t& rhv) const {
      return d_.count() < rhv.d_.count();
    }

   private:
    D d_;
  };

  class steady_clock {
   public:
    typedef time_point_t<steady_clock, milliseconds> time_point;

   public:
    static
    time_point now();
  };

  template<typename Clock>
  class block_duration {
   public:
    block_duration(milliseconds& duration) : duration_(duration) {
      start_ = Clock::now();
    }

    ~block_duration() {
      duration_ = Clock::now() - start_;
    }

   private:
    typename Clock::time_point start_;
    milliseconds& duration_;
  };
}

#endif

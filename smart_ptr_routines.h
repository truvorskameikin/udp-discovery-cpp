#ifndef __SMART_PTR_ROUTINES_H_
#define __SMART_PTR_ROUTINES_H_

#include <algorithm>

namespace smartptrroutines {
  template<typename T>
  class shared_ptr {
   public:
    typedef T element_type;

   public:
    shared_ptr() : ref_count_(0), value_(0) {
    }

    explicit
    shared_ptr(element_type* ptr) {
      if (ptr != 0) {
        ref_count_ = new long;
        (*ref_count_) = 1;

        value_ = ptr;
      }
    }

    shared_ptr(const shared_ptr& rhv)
        : ref_count_(rhv.ref_count_),
          value_(rhv.value_) {
      inc();
    }

    template<typename U>
    shared_ptr(const shared_ptr<U>& rhv)
        : ref_count_(rhv.ref_count_),
          value_(rhv.value_) {
      inc();
    }

#if defined(__cpp_rvalue_references)
    shared_ptr(shared_ptr&& rhv)
        : ref_count_(std::move(rhv.ref_count_)),
          value_(std::move(rhv.value_)) {
      rhv.clear();
    }

    template<typename U>
    shared_ptr(shared_ptr<U>&& rhv)
        : ref_count_(std::move(rhv.ref_count_)),
          value_(std::move(rhv.value_)) {
      rhv.clear();
    }
#endif

    ~shared_ptr() {
      destroy();
    }

    shared_ptr& operator=(const shared_ptr& rhv) {
      ref_count_ = rhv.ref_count_;
      value_ = rhv.value_;
      inc();

      return (*this);
    }

    template<typename U>
    shared_ptr<element_type>& operator=(const shared_ptr<U>& rhv) {
      ref_count_ = rhv.ref_count_;
      value_ = rhv.value_;
      inc();

      return (*this);
    }

#if defined(__cpp_rvalue_references)
    shared_ptr& operator=(shared_ptr&& rhv) {
      ref_count_ = std::move(rhv.ref_count_);
      value_ = std::move(rhv.value_);
      rhv.clear();

      return (*this);
    }

    template<typename U>
    shared_ptr<element_type>& operator=(shared_ptr<U>&& rhv) {
      ref_count_ = std::move(rhv.ref_count_);
      value_ = std::move(rhv.value_);
      rhv.clear();

      return (*this);
    }
#endif

    template<typename U>
    void swap(shared_ptr<U>& rhv) {
      std::swap(ref_count_, rhv.ref_count_);
      std::swap(value_, rhv.value_);
    }

    operator bool() const {
      return value_ != 0;
    }

    element_type& operator*() const {
      return (*value_);
    }

    element_type* operator->() const {
      return value_;
    }

    element_type* get() const {
      return value_;
    }

    long use_count() {
      if (!ref_count_)
        return 0;
      return (*ref_count_);
    }

    void reset() {
      destroy();
    }

   private:
    void inc() {
      if (ref_count_)
        ++(*ref_count_);
    }

    void clear() {
      ref_count_ = 0;
      value_ = 0;
    }

    void destroy() {
      if (!ref_count_)
        return;
      --(*ref_count_);
      if (*ref_count_ <= 0) {
        delete ref_count_;
        delete value_;
      }

      ref_count_ = 0;
      value_ = 0;
    }

   private:
    template<typename U>
    friend class shared_ptr;

    long* ref_count_;
    element_type* value_;
  };

  template<typename Type>
  shared_ptr<Type> make_shared() {
    return shared_ptr<Type>(new Type);
  }

  template<typename Type, typename Arg1>
  shared_ptr<Type> make_shared(const Arg1& arg1) {
    return shared_ptr<Type>(new Type(arg1));
  }

  template<typename Type, typename Arg1, typename Arg2>
  shared_ptr<Type> make_shared(const Arg1& arg1, const Arg2& arg2) {
    return shared_ptr<Type>(new Type(arg1, arg2));
  }

  template<typename Type, typename Arg1, typename Arg2, typename Arg3>
  shared_ptr<Type> make_shared(const Arg1& arg1, const Arg2& arg2, const Arg3& arg3) {
    return shared_ptr<Type>(new Type(arg1, arg2, arg3));
  }

  template<typename Type, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
  shared_ptr<Type> make_shared(const Arg1& arg1, const Arg2& arg2, const Arg3& arg3, const Arg4& arg4) {
    return shared_ptr<Type>(new Type(arg1, arg2, arg3, arg4));
  }

  template<typename Type, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
  shared_ptr<Type> make_shared(const Arg1& arg1, const Arg2& arg2, const Arg3& arg3, const Arg4& arg4, const Arg5& arg5) {
    return shared_ptr<Type>(new Type(arg1, arg2, arg3, arg4, arg5));
  }

  template<typename Type, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5, typename Arg6>
  shared_ptr<Type> make_shared(const Arg1& arg1, const Arg2& arg2, const Arg3& arg3, const Arg4& arg4, const Arg5& arg5, const Arg6& arg6) {
    return shared_ptr<Type>(new Type(arg1, arg2, arg3, arg4, arg5, arg6));
  }

  template<typename Type>
  const shared_ptr<Type> make_shared_null() {
    return shared_ptr<Type>();
  }
};

namespace std {
  template<typename T>
  void swap(smartptrroutines::shared_ptr<T>& lhv, smartptrroutines::shared_ptr<T>& rhv) {
    lhv.swap(rhv);
  }
};

#endif
#ifndef __UDP_DISCOVERY_CLIENT_H_
#define __UDP_DISCOVERY_CLIENT_H_

#include <stdint.h>
#include <string>

namespace udpdiscovery {
  namespace impl {
    class ClientWorkingEnvInterface {
     public:
      virtual ~ClientWorkingEnvInterface() {
      }

      virtual void SetUserData(const std::string& user_data) = 0;

      virtual void Exit() = 0;
    };
  };

  class Client {
   public:
    Client();
    ~Client();

    bool Start(int port, uint64_t application_id, const std::string& user_data);

    void SetUserData(const std::string& user_data);

    void Stop();

   private:
    bool started_;
    impl::ClientWorkingEnvInterface* working_env_;
  };
};

#endif

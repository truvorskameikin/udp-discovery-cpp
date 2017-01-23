#ifndef __UDP_DISCOVERY_CLIENT_H_
#define __UDP_DISCOVERY_CLIENT_H_

#include <string>

namespace udpdiscovery {
  namespace impl {
    class ClientSocketInterface {
     public:
      virtual ~ClientSocketInterface() {
      }

      virtual bool Create(int port) = 0;

      virtual int Send(const char* buffer, int buffer_size) = 0;
    };
  };

  class Client {
   public:
    Client();
    ~Client();

    bool Start(int port, const std::string& user_data);

    const std::string user_data() const {
      return user_data_;
    }

    void set_user_data(const std::string& user_data) {
      user_data_ = user_data;
    }

    void Send();

   private:
    bool started_;
    std::string user_data_;
    uint64_t packet_index_;
    uint64_t max_packet_index_;
    impl::ClientSocketInterface* sock_;
  };
};

#endif

#ifndef __UDP_DISCOVERY_SERVER_H_
#define __UDP_DISCOVERY_SERVER_H_

#include <list>
#include "discovered_client.hpp"

namespace udpdiscovery {
  namespace impl {
    class ServerWorkingEnvInterface {
     public:
      virtual ~ServerWorkingEnvInterface() {
      }

      virtual std::list<DiscoveredClient> ListClients() = 0;

      virtual void Exit() = 0;
    };
  };

  class Server {
   public:
    Server();
    ~Server();

    bool Start(int port);

    std::list<DiscoveredClient> ListClients() const;

    void Stop();

   private:
    bool started_;
    impl::ServerWorkingEnvInterface* working_env_;
  };

  bool Same(const IpPort& lhv, const IpPort& rhv);
  bool Same(const std::list<DiscoveredClient>& lhv, const std::list<DiscoveredClient>& rhv);
};

#endif

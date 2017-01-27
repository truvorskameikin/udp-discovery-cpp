#ifndef __UDP_DISCOVERY_ENDPOINT_H_
#define __UDP_DISCOVERY_ENDPOINT_H_

#include <list>
#include "discovered_endpoint.hpp"
#include "endpoint_parameters.hpp"

namespace udpdiscovery {
  namespace impl {
    class EndpointEnvInterface {
     public:
      virtual ~EndpointEnvInterface() {
      }

      virtual void SetUserData(const std::string& user_data) = 0;

      virtual std::list<DiscoveredEndpoint> ListDiscovered() = 0;

      virtual void Exit() = 0;
    };

    class MinimalisticThreadInterface {
     public:
      virtual ~MinimalisticThreadInterface() {
      }

      virtual void Detach() = 0;

      virtual void Join() = 0;
    };
  };

  class Endpoint {
   public:
    Endpoint();
    ~Endpoint();

    bool Start(const EndpointParameters& parameters, const std::string& user_data);

    void SetUserData(const std::string& user_data);

    std::list<DiscoveredEndpoint> ListDiscovered() const;

    void Stop(bool wait_for_thread);

   private:
    Endpoint(const Endpoint&);
    Endpoint& operator=(const Endpoint&);

   private:
    impl::EndpointEnvInterface* env_;
    impl::MinimalisticThreadInterface* thread_;
  };

  bool Same(EndpointParameters::SameEndpointMode mode, const IpPort& lhv, const IpPort& rhv);
  bool Same(EndpointParameters::SameEndpointMode mode, const std::list<DiscoveredEndpoint>& lhv, const std::list<DiscoveredEndpoint>& rhv);
};

#endif

/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#ifndef HELICS_CPP98_BROKER_HPP_
#define HELICS_CPP98_BROKER_HPP_
#pragma once

#include "../shared_api_library/helics.h"

#include <string>

namespace helics
{

class Broker
{
  public:
    // Default constructor, not meant to be used
    Broker () {};

    Broker (std::string type, std::string name, std::string initString)
    {
        broker = helicsCreateBroker (type.c_str(), name.c_str(), initString.c_str());
    }

    Broker (std::string type, std::string name, int argc, const char **argv)
    {
        broker = helicsCreateBrokerFromArgs (type.c_str(), name.c_str(), argc, argv);
    }

    Broker(const Broker &brk)
    {
        broker = helicsBrokerClone(brk.broker);
    }
    Broker &operator=(const Broker &brk)
    {
        broker = helicsBrokerClone(brk.broker);
        return *this;
    }
    virtual ~Broker ()
    {
        helicsBrokerFree (broker);
    }

    operator helics_broker() { return broker; }

    helics_broker baseObject() const { return broker; }

    bool isConnected () const
    {
        return helicsBrokerIsConnected (broker);
    }
    void disconnect()
    {
        helicsBrokerDisconnect(broker);
    }
    std::string getIdentifier() const
    {
        char str[255];
        helicsBrokerGetIdentifier(broker, &str[0], sizeof(str));
        std::string result(str);
        return result;
    }
    std::string getAddress() const
    {
        char str[255];
        helicsBrokerGetAddress(broker, &str[0], sizeof(str));
        std::string result(str);
        return result;
    }
  protected:
    helics_broker broker;
};

} //namespace helics
#endif

/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "Echo.hpp"
#include "PrecHelper.hpp"

#include <fstream>
#include <iostream>

#include "../common/JsonProcessingFunctions.hpp"
#include "../common/argParser.h"
#include "../core/helicsVersion.hpp"
#include <set>
#include <stdexcept>

namespace helics
{
namespace apps
{
static const ArgDescriptors InfoArgs{{"delay", "the delay with which the echo app will echo message"}};

Echo::Echo (int argc, char *argv[]) : App ("echo", argc, argv)
{
    if (!deactivated)
    {
        variable_map vm_map;
        argumentParser(argc, argv, vm_map, InfoArgs);
        loadArguments(vm_map);
        if (!masterFileName.empty())
        {
            loadFile(masterFileName);
        }
    }
}

Echo::Echo (const FederateInfo &fi) : App (fi) {}

Echo::Echo (const std::shared_ptr<Core> &core, const FederateInfo &fi) : App (core, fi) {}

Echo::Echo (const std::string &jsonString) : App (jsonString) { loadJsonFile (jsonString); }

void Echo::runTo (Time stopTime_input)
{
    auto state = fed->getCurrentState ();
    if (state == Federate::op_states::startup)
    {
        initialize ();
    }
    if (state < Federate::op_states::execution)
    {
        fed->enterExecutionState ();
    }
    else if (state == Federate::op_states::finalize)
    {
        return;
    }
    auto ctime = fed->getCurrentTime ();
    while (ctime < stopTime_input)
    {
        ctime = fed->requestTime (stopTime_input);
    }
}

void Echo::setEchoDelay (Time delay)
{
    std::lock_guard<std::mutex> lock (delayTimeLock);
    delayTime = delay;
}

void Echo::echoMessage (const Endpoint *ept, Time currentTime)
{
    auto m = ept->getMessage ();
    std::lock_guard<std::mutex> lock (delayTimeLock);
    while (m)
    {
        ept->send (m->original_source, m->data, currentTime + delayTime);
        m = ept->getMessage ();
    }
}

void Echo::addEndpoint (const std::string &endpointName, const std::string &endpointType)
{
    endpoints.emplace_back (GLOBAL, fed.get (), endpointName, endpointType);
    endpoints.back ().setCallback (
      [this](const Endpoint *ept, Time messageTime) { echoMessage (ept, messageTime); });
}

int Echo::loadArguments (boost::program_options::variables_map &vm_map)
{
    if (vm_map.count ("delay") > 0)
    {
        std::lock_guard<std::mutex> lock (delayTimeLock);
        delayTime = loadTimeFromString (vm_map["delay"].as<std::string> ());
    }
    return 0;
}

void Echo::loadJsonFile (const std::string &jsonFile)
{
    loadJsonFileConfiguration ("echo", jsonFile);
    auto eptCount = fed->getEndpointCount ();
    for (int ii = 0; ii < eptCount; ++ii)
    {
        endpoints.emplace_back (fed.get (), ii);
        endpoints.back ().setCallback (
          [this](const Endpoint *ept, Time messageTime) { echoMessage (ept, messageTime); });
    }

    auto doc = loadJsonString (jsonFile);

    if (doc.isMember ("echo"))
    {
        auto echoConfig = doc["echo"];

        if (echoConfig.isMember ("delay"))
        {
            std::lock_guard<std::mutex> lock (delayTimeLock);
            delayTime = loadJsonTime (echoConfig["delay"]);
        }
    }
}

}  // namespace apps
}  // namespace helics

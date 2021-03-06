/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/

#include "../application_api/Filters.hpp"
#include "../application_api/Subscriptions.hpp"
#include "../application_api/ValueFederate.hpp"
#include "../application_api/queryFunctions.hpp"
#include "../common/stringOps.h"
#include "../core/helicsVersion.hpp"
#include <algorithm>
#include <map>
#include <memory>
#include <iostream>
#include <regex>
#include <stdexcept>
#include <boost/filesystem.hpp>
#include "../common/argParser.h"

#include "../common/JsonProcessingFunctions.hpp"
#include "PrecHelper.hpp"
#include "Tracer.hpp"
#include <thread>
#include <boost/format.hpp>
#include "../common/logger.h"


namespace filesystem = boost::filesystem;

namespace helics
{
namespace apps
{
Tracer::Tracer(FederateInfo &fi) : App(fi)
{
    fed->setFlag(OBSERVER_FLAG);
}

static const ArgDescriptors InfoArgs{
    {"tags",ArgDescriptor::arg_type_t::vector_string,"tags to record, this argument may be specified any number of times"},
    { "endpoints",ArgDescriptor::arg_type_t::vector_string,"endpoints to capture, this argument may be specified multiple time" },
    {"sourceclone", ArgDescriptor::arg_type_t::vector_string, "existing endpoints to capture generated packets from, this argument may be specified multiple time"},
    {"destclone", ArgDescriptor::arg_type_t::vector_string, "existing endpoints to capture all packets with the specified endpoint as a destination, this argument may be specified multiple time"},
    {"clone", ArgDescriptor::arg_type_t::vector_string, "existing endpoints to clone all packets to and from"},
    { "allow_iteration", ArgDescriptor::arg_type_t::flag_type,"allow iteration on values" },
    {"capture", ArgDescriptor::arg_type_t::vector_string,"capture all the publications and endpoints of a particular federate capture=\"fed1;fed2\"  supports multiple arguments or a comma separated list"},
};

Tracer::Tracer(int argc, char *argv[]):App("tracer",argc,argv)
{
    if (!deactivated)
    {
        fed->setFlag(OBSERVER_FLAG);
        variable_map vm_map;
        argumentParser(argc, argv, vm_map, InfoArgs);
        loadArguments(vm_map);
        if (!masterFileName.empty())
        {
            loadFile(masterFileName);
        }
    }
}

Tracer::Tracer(const std::shared_ptr<Core> &core, const FederateInfo &fi):App(core,fi)
{
    fed->setFlag(OBSERVER_FLAG);
}

Tracer::Tracer(const std::string &jsonString) : App(jsonString)
{
    fed->setFlag(OBSERVER_FLAG);
    loadJsonFile(jsonString);
}

Tracer::~Tracer() = default;

void Tracer::loadJsonFile(const std::string &jsonString)
{
    loadJsonFileConfiguration("tracer",jsonString);

    auto subCount = fed->getSubscriptionCount ();
    for (int ii = 0; ii < subCount; ++ii)
    {
        subscriptions.emplace_back(fed.get(), ii);
        subkeys.emplace(subscriptions.back().getName(), static_cast<int> (subscriptions.size()) - 1);
    }
    auto eptCount = fed->getEndpointCount();
    for (int ii = 0; ii < eptCount; ++ii)
    {
        endpoints.emplace_back(fed.get(), ii);
        eptNames[endpoints.back().getName()] = static_cast<int> (endpoints.size() - 1);
    }

    auto doc = loadJsonString(jsonString);

    auto tags = doc["tag"];
    if (tags.isArray())
    {
        for (const auto &tag : tags)
        {
            addSubscription(tag.asString());
        }
    }
    else if (tags.isString())
    {
        addSubscription(tags.asString());
    }
    auto sourceClone = doc["sourceclone"];
    if (sourceClone.isArray())
    {
        for (const auto &sc : sourceClone)
        {
            addSourceEndpointClone(sc.asString());
        }
    }
    else if (sourceClone.isString())
    {
        addSourceEndpointClone(sourceClone.asString());
    }
    auto destClone = doc["destclone"];
    if (destClone.isArray())
    {
        for (const auto &dc : destClone)
        {
            addDestEndpointClone(dc.asString());
        }
    }
    else if (destClone.isString())
    {
        addDestEndpointClone(destClone.asString());
    }
    auto clones = doc["clone"];
    if (clones.isArray())
    {
        for (const auto &clone : clones)
        {
            addSourceEndpointClone(clone.asString());
            addDestEndpointClone(clone.asString());
        }
    }
    else if (clones.isString())
    {
        addSourceEndpointClone(clones.asString());
        addDestEndpointClone(clones.asString());
    }
    auto captures = doc["capture"];
    if (captures.isArray())
    {
        for (const auto &capture : captures)
        {
            addCapture(capture.asString());
        }
    }
    else if (captures.isString())
    {
        addCapture(captures.asString());
    }

}

void Tracer::loadTextFile(const std::string &textFile)
{
    using namespace stringOps;
    App::loadTextFile(textFile);
    std::ifstream infile(textFile);
    std::string str;
    int lc = 0;
    while (std::getline(infile, str))
    {
        ++lc;
        if (str.empty())
        {
            continue;
        }
        auto fc = str.find_first_not_of(" \t\n\r\0");
        if ((fc == std::string::npos) || (str[fc] == '#') || (str[fc] == '!'))
        {
            continue;
        }
        auto blk = splitlineQuotes(str, ",\t ", default_quote_chars, delimiter_compression::on);

        switch (blk.size())
        {
        case 1:
            addSubscription(removeQuotes(blk[0]));
            break;
        case 2:
            if ((blk[0] == "subscription") || (blk[0] == "s") || (blk[0] == "sub") || (blk[0] == "tag"))
            {
                addSubscription(removeQuotes(blk[1]));
            }
            else if ((blk[0] == "endpoint") || (blk[0] == "ept") || (blk[0] == "e"))
            {
                addEndpoint(removeQuotes(blk[1]));
            }
            else if ((blk[0] == "sourceclone") || (blk[0] == "source") || (blk[0] == "src"))
            {
                addSourceEndpointClone(removeQuotes(blk[1]));
            }
            else if ((blk[0] == "destclone") || (blk[0] == "dest") || (blk[0] == "destination"))
            {
                addDestEndpointClone(removeQuotes(blk[1]));
            }
            else if (blk[0] == "capture")
            {
                addCapture(removeQuotes(blk[1]));
            }
            else if (blk[0] == "clone")
            {
                addSourceEndpointClone(removeQuotes(blk[1]));
                addDestEndpointClone(removeQuotes(blk[1]));
            }
            else
            {
                std::cerr << "Unable to process line " << lc << ':' << str << '\n';
            }
            break;
        case 3:
            if (blk[0] == "clone")
            {
                if ((blk[1] == "source") || (blk[1] == "src"))
                {
                    addSourceEndpointClone(removeQuotes(blk[2]));
                }
                else if ((blk[1] == "dest") || (blk[1] == "destination"))
                {
                    addDestEndpointClone(removeQuotes(blk[2]));
                }
                else
                {
                    std::cerr << "Unable to process line " << lc << ':' << str << '\n';
                }
            }
            else
            {
                std::cerr << "Unable to process line " << lc << ':' << str << '\n';
            }
            break;
        default:
            break;
        }
    }
    infile.close();
}

void Tracer::initialize()
{
    auto state = fed->getCurrentState();
    if (state == Federate::op_states::startup)
    {
        generateInterfaces();

        fed->enterInitializationState();
        captureForCurrentTime(-1.0);
    }
}

void Tracer::generateInterfaces()
{
    for (auto &tag : subkeys)
    {
        if (tag.second == -1)
        {
            addSubscription(tag.first);
        }
    }

    loadCaptureInterfaces();
}

void Tracer::loadCaptureInterfaces()
{
    for (auto &capt : captureInterfaces)
    {
        auto res = waitForInit(fed.get(), capt);
        if (res)
        {
            auto pubs = vectorizeQueryResult(fed->query(capt, "publications"));
            for (auto &pub : pubs)
            {
                addSubscription(pub);
            }
        }
    }
}

void Tracer::captureForCurrentTime(Time currentTime, int iteration)
{
    static auto logger = LoggerManager::getLoggerCore();
    for (auto &sub : subscriptions)
    {
        if (sub.isUpdated())
        {
            auto val = sub.getValue<std::string>();

            if (printMessage)
            {
                std::string valstr;
                if (val.size() < 150)
                {
                    if (iteration > 0)
                    {
                        valstr = (boost::format("[%f:%d]value %s=%s") % currentTime %iteration% sub.getKey() % val).str();
                    }
                    else
                    {
                        valstr = (boost::format("[%f]value %s=%s") % currentTime % sub.getKey() % val).str();
                    }
                }
                else
                {
                    if (iteration > 0)
                    {
                        valstr = (boost::format("[%f:%d]value %s=block[%d]") % currentTime %iteration% sub.getKey() % val.size()).str();
                    }
                    else
                    {
                        valstr = (boost::format("[%f]value %s=block[%d]") % currentTime % sub.getKey() % val.size()).str();
                    }
                   
                }
                logger->addMessage(std::move(valstr));
            }
            if (valueCallback)
            {
                valueCallback(currentTime, sub.getKey(), val);
            }

        }
    }

    for (auto &ept : endpoints)
    {
        while (ept.hasMessage())
        {
            auto mess = ept.getMessage();
            if (printMessage)
            {
                std::string messstr;
                if (mess->data.size() < 50)
                {
                    messstr = (boost::format("[%f]message from %s to %s::%s") % currentTime % mess->source % mess->dest %mess->data.to_string()).str();
                }
                else
                {
                    messstr = (boost::format("[%f]message from %s to %s:: size %d") % currentTime % mess->source % mess->dest %mess->data.size()).str();
                }
                logger->addMessage(std::move(messstr));
            }
            if (endpointMessageCallback)
            {
                endpointMessageCallback(currentTime, ept.getName(), std::move(mess));
            }
        }
    }

    // get the clone endpoints
    if (cloneEndpoint)
    {
        while (cloneEndpoint->hasMessage())
        {
            auto mess = cloneEndpoint->getMessage();
            if (printMessage)
            {
                std::string messstr;
                if (mess->data.size() < 50)
                {
                    messstr = (boost::format("[%f]message from %s to %s::%s") % currentTime % mess->source % mess->original_dest %mess->data.to_string()).str();
                }
                else
                {
                    messstr = (boost::format("[%f]message from %s to %s:: size %d") % currentTime % mess->source % mess->original_dest %mess->data.size()).str();
                }
                logger->addMessage(std::move(messstr));
            }
            if (clonedMessageCallback)
            {
                clonedMessageCallback(currentTime, std::move(mess));
            }
        }
    }
}


/** run the Player until the specified time*/
void Tracer::runTo(Time runToTime)
{
    auto state = fed->getCurrentState();
    if (state == Federate::op_states::startup)
    {
        initialize();
        state = Federate::op_states::initialization;
    }

    if (state == Federate::op_states::initialization)
    {
        fed->enterExecutionState();
        captureForCurrentTime(0.0);
    }


    

    Time nextPrintTime = 10.0;
    try
    {
        while (true)
        {
            helics::Time T;
            int iteration = 0;
            if (allow_iteration)
            {
                auto ItRes = fed->requestTimeIterative(runToTime, iteration_request::iterate_if_needed);
                if (ItRes.state == iteration_result::next_step)
                {
                    iteration = 0;
                }
                T = ItRes.grantedTime;
                captureForCurrentTime(T,iteration);
                ++iteration;
            }
            else
            {
                T = fed->requestTime(runToTime);
                captureForCurrentTime(T);
            }
            if (T >= runToTime)
            {
                break;
            }
            if (T >= nextPrintTime)
            {
                std::cout << "processed for time " << static_cast<double> (T) << "\n";
                nextPrintTime += 10.0;
            }
        }
    }
    catch (...)
    {
    }
}
/** add a subscription to record*/
void Tracer::addSubscription(const std::string &key)
{
    auto res = subkeys.find(key);
    if ((res == subkeys.end()) || (res->second == -1))
    {
        subscriptions.push_back(helics::Subscription(fed.get(), key));
        auto index = static_cast<int> (subscriptions.size()) - 1;
        subkeys[key] = index;  // this is a potential replacement
    }
}


/** add an endpoint*/
void Tracer::addEndpoint(const std::string &endpoint)
{
    auto res = eptNames.find(endpoint);
    if ((res == eptNames.end()) || (res->second == -1))
    {
        endpoints.push_back(helics::Endpoint(GLOBAL, fed.get(), endpoint));
        auto index = static_cast<int> (endpoints.size()) - 1;
        eptNames[endpoint] = index;  // this is a potential replacement
    }
}
void Tracer::addSourceEndpointClone(const std::string &sourceEndpoint)
{
    if (!cFilt)
    {
        cFilt = std::make_unique<CloningFilter>(fed.get());
        cloneEndpoint = std::make_unique<Endpoint>(fed.get(), "cloneE");
        cFilt->addDeliveryEndpoint(cloneEndpoint->getName());
    }
    cFilt->addSourceTarget(sourceEndpoint);
}

void Tracer::addDestEndpointClone(const std::string &destEndpoint)
{
    if (!cFilt)
    {
        cFilt = std::make_unique<CloningFilter>(fed.get());
        cloneEndpoint = std::make_unique<Endpoint>(fed.get(), "cloneE");
        cFilt->addDeliveryEndpoint(cloneEndpoint->getName());
    }
    cFilt->addDestinationTarget(destEndpoint);
}

void Tracer::addCapture(const std::string &captureDesc) { captureInterfaces.push_back(captureDesc); }

int Tracer::loadArguments(boost::program_options::variables_map &vm_map)
{
    if (vm_map.count("input") == 0)
    {
        return -1;
    }

    if (!filesystem::exists(vm_map["input"].as<std::string>()))
    {
        std::cerr << vm_map["input"].as<std::string>() << "is not a valid input file \n";
        return -3;
    }
    loadFile(vm_map["input"].as<std::string>());

    // get the extra tags from the arguments
    if (vm_map.count("tags") > 0)
    {
        auto argTags = vm_map["tags"].as<std::vector<std::string>>();
        for (const auto &tag : argTags)
        {
            auto taglist = stringOps::splitlineQuotes(tag);
            for (const auto &tagname : taglist)
            {
                subkeys.emplace(stringOps::removeQuotes(tagname), -1);
            }
        }
    }

    // capture the all the publications from a particular federate
    if (vm_map.count("capture") > 0)
    {
        auto captures = vm_map["capture"].as<std::vector<std::string>>();
        for (const auto &capt : captures)
        {
            auto captFeds = stringOps::splitlineQuotes(capt);
            for (auto &captFed : captFeds)
            {
                auto actCapt = stringOps::removeQuotes(captFed);
                captureInterfaces.push_back(actCapt);
            }
        }
    }

    if (vm_map.count("clone") > 0)
    {
        auto clones = vm_map["clone"].as<std::vector<std::string>>();
        for (const auto &clone : clones)
        {
            addDestEndpointClone(clone);
            addSourceEndpointClone(clone);
        }
    }

    if (vm_map.count("sourceclone") > 0)
    {
        auto clones = vm_map["sourceclone"].as<std::vector<std::string>>();
        for (const auto &clone : clones)
        {
            addSourceEndpointClone(clone);
        }
    }

    if (vm_map.count("destclone") > 0)
    {
        auto clones = vm_map["destclone"].as<std::vector<std::string>>();
        for (const auto &clone : clones)
        {
            addDestEndpointClone(clone);
        }
    }
    if (vm_map.count("allow_iteration") > 0)
    {
        allow_iteration = true;
    }
    return 0;
}
}  // namespace apps
} // namespace helics


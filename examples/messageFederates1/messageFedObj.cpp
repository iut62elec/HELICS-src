/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "helics/application_api/Endpoints.hpp"
#include <iostream>
#include <thread>
#include "helics/core/BrokerFactory.hpp"
#include "helics/common/argParser.h"

static const helics::ArgDescriptors InfoArgs{
    {"startbroker","start a broker with the specified arguments"},
    {"target,t", "name of the target federate"},
    {"endpoint,e", "name of the target endpoint"},
    {"source,s", "name of the source endpoint"}
    //name is captured in the argument processor for federateInfo
};

int main (int argc, char *argv[])
{
    helics::FederateInfo fi("fed");
    helics::variable_map vm;
    auto parseResult = argumentParser(argc, argv, vm, InfoArgs);
    fi.loadInfoFromArgs(argc, argv);
    if (parseResult != 0)
    {
        return 0;
    }

	std::string targetfederate = "fed";
	if (vm.count("target") > 0)
	{
		targetfederate = vm["target"].as<std::string>();
	}
    std::string targetEndpoint = "endpoint";
    if (vm.count("endpoint") > 0) {
        targetEndpoint = vm["endpoint"].as<std::string>();
    }
    std::string target = targetfederate + "/" + targetEndpoint;
    std::string myendpoint = "endpoint";
    if (vm.count("source") > 0)
    {
        myendpoint = vm["source"].as<std::string>();
    }

    fi.logLevel = 5;
    std::shared_ptr<helics::Broker> brk;
    if (vm.count("startbroker") > 0)
    {
        brk = helics::BrokerFactory::create(fi.coreType, vm["startbroker"].as<std::string>());
    }

    auto mFed = std::make_unique<helics::MessageFederate> (fi);
    auto name = mFed->getName();
	std::cout << " registering endpoint '" << myendpoint << "' for " << name<<'\n';

    // create the endpoint using the Endpoint object interface
    helics::Endpoint endpoint(mFed.get(), myendpoint);


    std::cout << "entering init State\n";
    mFed->enterInitializationState ();
    std::cout << "entered init State\n";
    mFed->enterExecutionState ();
    std::cout << "entered exec State\n";
    // set a defined target for the endpoint so it doesn't have to specfied on every call
    endpoint.setTargetDestination(target);
    for (int i=1; i<10; ++i) {
		std::string message = "message sent from "+name+" to "+target+" at time " + std::to_string(i);
		endpoint.send(message.data(), message.size());
        std::cout << message << std::endl;
        auto newTime = mFed->requestTime (i);
		std::cout << "processed time " << static_cast<double> (newTime) << "\n";
		while (endpoint.hasMessage())
		{
			auto nmessage = endpoint.getMessage();
			std::cout << "received message from " << nmessage->source << " at " << static_cast<double>(nmessage->time) << " ::" << nmessage->data.to_string() << '\n';
		}

    }
    mFed->finalize ();
    if (brk)
    {
        while (brk->isConnected())
        {
            std::this_thread::yield();
        }
        brk = nullptr;
    }
    return 0;
}


/*
Copyright (C) 2017-2018, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.
*/

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include "ctestFixtures.hpp"
#include "test_configuration.h"

#include <future>
#include <iostream>
#include <thread>
// these test cases test out the message federates


BOOST_FIXTURE_TEST_SUITE (message_federate_tests, FederateTestFixture)

namespace bdata = boost::unit_test::data;
#if ENABLE_TEST_TIMEOUTS > 0
namespace utf = boost::unit_test;
#endif

/** test simple creation and destruction*/
BOOST_DATA_TEST_CASE (message_federate_initialize_tests, bdata::make (core_types_single), core_type)
{
    SetupTest(helicsCreateMessageFederate,core_type, 1);
    auto mFed1 = GetFederateAt(0);

    CE (helicsFederateEnterExecutionMode (mFed1));

    federate_state mFed1State;
    CE(helicsFederateGetState(mFed1, &mFed1State));
    BOOST_CHECK (mFed1State == helics_execution_state);

    CE (helicsFederateFinalize(mFed1));

   CE(helicsFederateGetState(mFed1, &mFed1State));
    BOOST_CHECK (mFed1State == federate_state::helics_finalize_state);
}

BOOST_DATA_TEST_CASE (message_federate_endpoint_registration, bdata::make (core_types_single), core_type)
{
	SetupTest(helicsCreateMessageFederate,core_type, 1);
	auto mFed1 = GetFederateAt(0);

    auto epid = helicsFederateRegisterEndpoint (mFed1, "ep1", NULL);
    auto epid2 = helicsFederateRegisterGlobalEndpoint (mFed1, "ep2", "random");

    CE(helicsFederateEnterExecutionMode (mFed1));

    federate_state mFed1State = helics_execution_state;
   CE(helicsFederateGetState(mFed1, &mFed1State));
    BOOST_CHECK (mFed1State == helics_execution_state);
    char sv[32];
    char sv2[32];
    CE(helicsEndpointGetName (epid, sv, 32));
    CE(helicsEndpointGetName (epid2, sv2, 32));
    BOOST_CHECK_EQUAL (sv, "fed0/ep1");
    BOOST_CHECK_EQUAL (sv2, "ep2");

    CE(helicsEndpointGetType (epid, sv, 32));
    CE(helicsEndpointGetType (epid2, sv2, 32));
    BOOST_CHECK_EQUAL (sv, "");
    BOOST_CHECK_EQUAL (sv2, "random");

    CE( helicsFederateFinalize(mFed1));

	mFed1State = federate_state::helics_finalize_state;
	CE(helicsFederateGetState(mFed1, &mFed1State));
	BOOST_CHECK (mFed1State == federate_state::helics_finalize_state);
}

#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (5))
#endif
BOOST_DATA_TEST_CASE (message_federate_send_receive, bdata::make (core_types_single), core_type)
{
	SetupTest(helicsCreateMessageFederate,core_type, 1);
	auto mFed1 = GetFederateAt(0);

    auto epid = helicsFederateRegisterEndpoint (mFed1, "ep1", NULL);
    auto epid2 = helicsFederateRegisterGlobalEndpoint (mFed1, "ep2", "random");
    CE(helicsFederateSetTimeDelta (mFed1, 1.0));

    CE(helicsFederateEnterExecutionMode (mFed1));

    federate_state mFed1State = helics_execution_state;
    CE(helicsFederateGetState(mFed1, &mFed1State));
    BOOST_CHECK (mFed1State == helics_execution_state);
    std::string data(500, 'a');

    CE(helicsEndpointSendEventRaw (epid, "ep2", data.c_str(), 500, 0.0));
    helics_time_t time;
    CE(helicsFederateRequestTime (mFed1, 1.0, &time));
    BOOST_CHECK_EQUAL (time, 1.0);

    auto res = helicsFederateHasMessage (mFed1);
    BOOST_CHECK (res);
    res = helicsEndpointHasMessage (epid);
    BOOST_CHECK (res == false);
    res = helicsEndpointHasMessage (epid2);
    BOOST_CHECK (res);

    auto M = helicsEndpointGetMessage (epid2);
    //BOOST_REQUIRE (M);
    BOOST_REQUIRE_EQUAL (M.length, 500);

    BOOST_CHECK_EQUAL (M.data[245], 'a');
    CE(helicsFederateFinalize(mFed1));

	mFed1State = federate_state::helics_finalize_state;
	CE(helicsFederateGetState(mFed1, &mFed1State));
	BOOST_CHECK (mFed1State == federate_state::helics_finalize_state);
}

#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (5))
#endif
BOOST_DATA_TEST_CASE (message_federate_send_receive_2fed, bdata::make (core_types), core_type)
{
   // extraBrokerArgs = "--logleve=4";
	SetupTest(helicsCreateMessageFederate,core_type, 2);
	auto mFed1 = GetFederateAt(0);
    auto mFed2 = GetFederateAt(1);
    //mFed1->setLoggingLevel(4);
    //mFed2->setLoggingLevel(4);
    auto epid = helicsFederateRegisterEndpoint (mFed1, "ep1", NULL);
    auto epid2 = helicsFederateRegisterGlobalEndpoint (mFed2, "ep2", "random");

    CE(helicsFederateSetTimeDelta (mFed1, 1.0));
     CE(helicsFederateSetTimeDelta (mFed2, 1.0));

     CE(helicsFederateEnterExecutionModeAsync (mFed1));
    CE(helicsFederateEnterExecutionMode (mFed2));
    CE(helicsFederateEnterExecutionModeComplete (mFed1));

    federate_state mFed1State = helics_execution_state;
    CE(helicsFederateGetState(mFed1, &mFed1State));
	BOOST_CHECK (mFed1State == helics_execution_state);
    federate_state mFed2State = helics_execution_state;
    CE(helicsFederateGetState(mFed2, &mFed2State));
	BOOST_CHECK (mFed2State == helics_execution_state);

	std::string data(500, 'a');
	std::string data2(400, 'b');

    CE(helicsEndpointSendEventRaw (epid, "ep2", data.c_str(), 500, 0.0));
    CE(helicsEndpointSendEventRaw (epid2, "fed0/ep1", data2.c_str(), 400, 0.0));
    // move the time to 1.0
	helics_time_t time;
    CE(helicsFederateRequestTimeAsync (mFed1, 1.0));
    helics_time_t gtime;
    CE(helicsFederateRequestTime (mFed2, 1.0, &gtime));
    CE(helicsFederateRequestTimeComplete (mFed1, &time));

    BOOST_CHECK_EQUAL (gtime, 1.0);
    BOOST_CHECK_EQUAL (time, 1.0);

    auto res = helicsFederateHasMessage (mFed1);
    BOOST_CHECK (res);
    res = helicsEndpointHasMessage (epid);
    BOOST_CHECK (res);
    res = helicsEndpointHasMessage (epid2);
    BOOST_CHECK (res);

    auto M1 = helicsEndpointGetMessage (epid);
    //BOOST_REQUIRE(M1);
    BOOST_REQUIRE_EQUAL (M1.length, 400);

    BOOST_CHECK_EQUAL (M1.data[245], 'b');

    auto M2 = helicsEndpointGetMessage (epid2);
    //BOOST_REQUIRE(M2);
    BOOST_REQUIRE_EQUAL (M2.length, 500);

    BOOST_CHECK_EQUAL (M2.data[245], 'a');
    CE(helicsFederateFinalize(mFed1));
    CE(helicsFederateFinalize(mFed2));

	mFed1State = federate_state::helics_finalize_state;
    CE(helicsFederateGetState(mFed1, &mFed1State));
	BOOST_CHECK (mFed1State == federate_state::helics_finalize_state);
	mFed2State = federate_state::helics_finalize_state;
    CE(helicsFederateGetState(mFed2, &mFed2State));
	BOOST_CHECK (mFed2State == federate_state::helics_finalize_state);
}

#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (5))
#endif
BOOST_DATA_TEST_CASE (message_federate_send_receive_2fed_multisend, bdata::make (core_types), core_type)
{
	SetupTest(helicsCreateMessageFederate,core_type, 2);
	auto mFed1 = GetFederateAt(0);
    auto mFed2 = GetFederateAt(1);

    auto epid = helicsFederateRegisterEndpoint (mFed1, "ep1", NULL);
    auto epid2 = helicsFederateRegisterGlobalEndpoint (mFed2, "ep2", "random");
    // mFed1->getCorePointer()->setLoggingLevel(0, 5);
    helicsFederateSetTimeDelta (mFed1, 1.0);
    helicsFederateSetTimeDelta (mFed2, 1.0);

    helicsFederateEnterExecutionModeAsync (mFed1);
    helicsFederateEnterExecutionMode (mFed2);
    helicsFederateEnterExecutionModeComplete (mFed1);

    federate_state mFed1State = helics_execution_state;
	helicsFederateGetState(mFed1, &mFed1State);
	BOOST_CHECK (mFed1State == helics_execution_state);
    federate_state mFed2State = helics_execution_state;
	helicsFederateGetState(mFed2, &mFed2State);
	BOOST_CHECK (mFed2State == helics_execution_state);

    std::string data1 (500, 'a');
    std::string data2 (400, 'b');
    std::string data3 (300, 'c');
    std::string data4 (200, 'd');
    helicsEndpointSendEventRaw (epid, "ep2", data1.c_str(), 500, 0.0);
    helicsEndpointSendEventRaw (epid, "ep2", data2.c_str(), 400, 0.0);
    helicsEndpointSendEventRaw (epid, "ep2", data3.c_str(), 300, 0.0);
    helicsEndpointSendEventRaw (epid, "ep2", data4.c_str(), 200, 0.0);
    // move the time to 1.0
	helics_time_t time;
    helicsFederateRequestTimeAsync (mFed1, 1.0);
    helics_time_t gtime;
    helicsFederateRequestTime (mFed2, 1.0, &gtime);
    helicsFederateRequestTimeComplete (mFed1, &time);

    BOOST_CHECK_EQUAL (gtime, 1.0);
    BOOST_CHECK_EQUAL (time, 1.0);

    auto res = helicsFederateHasMessage (mFed1);
    BOOST_CHECK (!res);

    res = helicsEndpointHasMessage (epid);
    BOOST_CHECK (!res);
    auto cnt = helicsEndpointReceiveCount (epid2);
    BOOST_CHECK_EQUAL (cnt, 4);

    auto M1 = helicsEndpointGetMessage (epid2);
    //BOOST_REQUIRE(M1);
    BOOST_REQUIRE_EQUAL (M1.length, 500);

    BOOST_CHECK_EQUAL (M1.data[245], 'a');
    // check the count decremented
    cnt = helicsEndpointReceiveCount (epid2);
    BOOST_CHECK_EQUAL (cnt, 3);
    auto M2 = helicsEndpointGetMessage (epid2);
    //BOOST_REQUIRE(M2);
    BOOST_REQUIRE_EQUAL (M2.length, 400);
    BOOST_CHECK_EQUAL (M2.data[245], 'b');
    cnt = helicsEndpointReceiveCount (epid2);
    BOOST_CHECK_EQUAL (cnt, 2);

    auto M3 = helicsEndpointGetMessage (epid2);
    auto M4 = helicsEndpointGetMessage (epid2);
    //BOOST_REQUIRE(M3);
    //BOOST_REQUIRE(M4);
    BOOST_CHECK_EQUAL (M3.length, 300);
    BOOST_CHECK_EQUAL (M4.length, 200);

    BOOST_CHECK_EQUAL (M4.source, "fed0/ep1");
    BOOST_CHECK_EQUAL (M4.dest, "ep2");
    BOOST_CHECK_EQUAL (M4.original_source, "fed0/ep1");
    BOOST_CHECK_EQUAL (M4.time, 0.0);
    helicsFederateFinalize(mFed1);
    helicsFederateFinalize(mFed2);

	mFed1State = federate_state::helics_finalize_state;
	helicsFederateGetState(mFed1, &mFed1State);
	BOOST_CHECK (mFed1State == federate_state::helics_finalize_state);
	mFed2State = federate_state::helics_finalize_state;
	helicsFederateGetState(mFed2, &mFed2State);
	BOOST_CHECK (mFed2State == federate_state::helics_finalize_state);
}

//#define ENABLE_OUTPUT
//trivial Federate that sends Messages and echoes a ping with a pong

class pingpongFed
{
  private:
	helics_federate mFed;
	helics_time_t delta;  // the minimum time delta for the federate
    std::string name;  //!< the name of the federate
    std::string coreType;
    std::vector<std::pair<helics_time_t, std::string>> triggers;
    helics_endpoint ep;
    int index = 0;

  public:
    int pings = 0;  //!< the number of pings received
    int pongs = 0;  //!< the number of pongs received
  public:
    pingpongFed (std::string fname, helics_time_t tDelta, std::string ctype)
        : delta (tDelta), name (std::move (fname)), coreType (ctype)
    {
        if (delta <= 0.0)
        {
            delta = 0.2;
        }
    }

  private:
    void initialize ()
    {
    	helics_federate_info_t fi = helicsFederateInfoCreate();
    	helicsFederateInfoSetFederateName(fi, name.c_str());
    	helicsFederateInfoSetCoreName(fi, "pptest");
    	helicsFederateInfoSetCoreTypeFromString(fi, coreType.c_str());
    	helicsFederateInfoSetCoreInitString(fi, "3");
    	helicsFederateInfoSetTimeDelta(fi, delta);
#ifdef ENABLE_OUTPUT
        std::cout << std::string ("about to create federate ") + name + "\n";
#endif
        mFed = helicsCreateMessageFederate(fi);
#ifdef ENABLE_OUTPUT
        std::cout << std::string ("registering federate ") + name + "\n";
#endif
        ep = helicsFederateRegisterEndpoint (mFed, "port", NULL);
    }

  private:
    void processMessages (helics_time_t currentTime)
    {
        while (helicsEndpointHasMessage (ep))
        {
            auto mess = helicsEndpointGetMessage (ep);
            std::string messString (mess.data);
            if (messString == "ping")
            {
#ifdef ENABLE_OUTPUT
                std::cout << name << " :receive ping from " << std::string (mess.source) << " at time "
                          << static_cast<double> (currentTime) << '\n';
#endif
                message_t replyMess;
                replyMess.data = "pong";
                replyMess.length = 4;
                replyMess.dest = mess.source;
                replyMess.source = name.c_str();
                replyMess.original_source = mess.dest;
                replyMess.original_dest = mess.source;
                replyMess.time = currentTime;
                helicsEndpointSendMessage (ep, &replyMess);
                pings++;
            }
            else if (messString == "pong")
            {
                pongs++;
#ifdef ENABLE_OUTPUT
                std::cout << name << " :receive pong from " << std::string (mess.source) << " at time "
                          << static_cast<double> (currentTime) << '\n';
#endif
            }
        }
    }
    void mainLoop (helics_time_t finish)
    {
    	helics_time_t nextTime = 0;
        while (nextTime <= finish)
        {
            processMessages (nextTime);
            if (index < static_cast<int> (triggers.size ()))
            {
                while (triggers[index].first <= nextTime)
                {
#ifdef ENABLE_OUTPUT
                    std::cout << name << ": send ping to " << triggers[index].second << " at time "
                              << static_cast<double> (nextTime) << '\n';
#endif
                    message_t mess;
                    mess.data = "ping";
                    mess.length = 4;
                    mess.dest = triggers[index].second.c_str();
                    mess.source = name.c_str();
                    mess.original_source = name.c_str();
                    mess.original_dest = triggers[index].second.c_str();
                    mess.time = triggers[index].first;
                    helicsEndpointSendMessage (ep, &mess);
                    ++index;
                    if (index >= static_cast<int> (triggers.size ()))
                    {
                        break;
                    }
                }
            }
            helics_time_t requestTime = nextTime + delta;
            helicsFederateRequestTime (mFed, requestTime, &nextTime);
        }
        helicsFederateFinalize (mFed);
    }

  public:
    void run (helics_time_t finish)
    {
        initialize ();
        helicsFederateEnterExecutionMode (mFed);
#ifdef ENABLE_OUTPUT
        std::cout << std::string ("entering Execute Mode ") + name + "\n";
#endif
        mainLoop (finish);
    }
    void addTrigger (helics_time_t triggerTime, const std::string &target)
    {
        triggers.emplace_back (triggerTime, target);
    }
};

#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (20))
#endif
BOOST_DATA_TEST_CASE (threefedPingPong, bdata::make (core_types), core_type)
{
    if (core_type != "test")
    {
        return;
    }
    AddBroker (core_type, "3");

    pingpongFed p1 ("fedA", 0.5, core_type);
    pingpongFed p2 ("fedB", 0.5, core_type);
    pingpongFed p3 ("fedC", 0.5, core_type);

    p1.addTrigger (0.5, "fedB/port");
    p1.addTrigger (0.5, "fedC/port");
    p1.addTrigger (3.0, "fedB/port");
    p2.addTrigger (1.5, "fedA/port");
    p3.addTrigger (3.0, "fedB/port");
    p3.addTrigger (4.0, "fedA/port");

    auto t1 = std::thread ([&p1]() { p1.run (6.0); });
    auto t2 = std::thread ([&p2]() { p2.run (6.0); });
    auto t3 = std::thread ([&p3]() { p3.run (6.0); });

    t1.join ();
    t2.join ();
    t3.join ();
    BOOST_CHECK_EQUAL (p1.pings, 2);
    BOOST_CHECK_EQUAL (p2.pings, 3);
    BOOST_CHECK_EQUAL (p3.pings, 1);
    BOOST_CHECK_EQUAL (p1.pongs, 3);
    BOOST_CHECK_EQUAL (p2.pongs, 1);
    BOOST_CHECK_EQUAL (p3.pongs, 2);
}

#if ENABLE_TEST_TIMEOUTS > 0
BOOST_TEST_DECORATOR (*utf::timeout (5))
#endif
BOOST_DATA_TEST_CASE (test_time_interruptions, bdata::make (core_types), core_type)
{
	SetupTest(helicsCreateMessageFederate,core_type, 2);
	auto mFed1 = GetFederateAt(0);
    auto mFed2 = GetFederateAt(1);

    auto epid = helicsFederateRegisterEndpoint (mFed1, "ep1", NULL);
    auto epid2 = helicsFederateRegisterGlobalEndpoint (mFed2, "ep2", "random");
    helicsFederateSetTimeDelta (mFed1, 1.0);
    helicsFederateSetTimeDelta (mFed2, 0.5);

    helicsFederateEnterExecutionModeAsync (mFed1);
    helicsFederateEnterExecutionMode (mFed2);
    helicsFederateEnterExecutionModeComplete (mFed1);

    federate_state mFed1State = helics_execution_state;
	helicsFederateGetState(mFed1, &mFed1State);
	BOOST_CHECK (mFed1State == helics_execution_state);
    federate_state mFed2State = helics_execution_state;
	helicsFederateGetState(mFed2, &mFed2State);
	BOOST_CHECK (mFed2State == helics_execution_state);

    std::string data1 (500, 'a');
    std::string data2 (400, 'b');

    helicsEndpointSendEventRaw (epid, "ep2", data1.c_str(), 500, 0.0);
	helicsEndpointSendEventRaw (epid2, "fed0/ep1", data2.c_str(), 400, 0.0);
    // move the time to 1.0
	helics_time_t time;
    helicsFederateRequestTimeAsync (mFed1, 1.0);
    helics_time_t gtime;
    helicsFederateRequestTime (mFed2, 1.0, &gtime);


    BOOST_REQUIRE_EQUAL (gtime, 0.5);

    auto res = helicsFederateHasMessage (mFed2);
    BOOST_CHECK (res);

    auto M2 = helicsEndpointGetMessage (epid2);
    BOOST_REQUIRE_EQUAL (M2.length, 500);

    BOOST_CHECK_EQUAL (M2.data[245], 'a');

    helicsFederateRequestTime (mFed2, 1.0, &gtime);
    helicsFederateRequestTimeComplete (mFed1, &time);
    BOOST_CHECK_EQUAL (gtime, 1.0);

    BOOST_CHECK_EQUAL (time, 1.0);
    auto M1 = helicsEndpointGetMessage (epid);
    //BOOST_CHECK (M1);
    BOOST_REQUIRE_EQUAL (M1.length, 400);
    BOOST_CHECK_EQUAL (M1.data[245], 'b');

    res = helicsFederateHasMessage (mFed1);
    BOOST_CHECK (!res);
    helicsFederateFinalize(mFed1);
    helicsFederateFinalize(mFed2);

	mFed1State = federate_state::helics_finalize_state;
	helicsFederateGetState(mFed1, &mFed1State);
	BOOST_CHECK (mFed1State == federate_state::helics_finalize_state);
	mFed2State = federate_state::helics_finalize_state;
	helicsFederateGetState(mFed2, &mFed2State);
	BOOST_CHECK (mFed2State == federate_state::helics_finalize_state);
}
BOOST_AUTO_TEST_SUITE_END ()


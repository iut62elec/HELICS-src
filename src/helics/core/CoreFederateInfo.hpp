/*
Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#pragma once

#include "helics-time.hpp"

namespace helics
{
/** class defining some required information about the federate*/
class CoreFederateInfo
{
  public:
    Time timeDelta = timeEpsilon;  // the minimum time advance allowed by the federate
    Time outputDelay =
      timeZero;  //!< the outputDelay value, the amount of time values and messages take to propagate to be
                 //!< available to external federates
    Time inputDelay =
      timeZero;  //!< the time it takes values and messages to propagate to be accessible to the Federate
    Time period = timeZero;  //!< a period value,  all granted times must be on this period n*Period+offset
    Time offset = timeZero;  //!< offset to the time period

    bool observer = false;  //!< flag indicating that the federate is an observer
    bool uninterruptible =
      false;  //!< flag indicating that the federate should never return a time other than requested
    bool source_only =
      false;  //!< flag indicating that the federate does not receive or do anything with received information.
    bool only_transmit_on_change =
      false;  //!< flag indicating that values should only updated if the number has actually changes
    bool only_update_on_change =
      false;  //!< flag indicating values should be discarded if they are not changed from previous values
    bool wait_for_current_time_updates = false;  //!< flag indicating that the federate should only grant when no
                                                 //!< more messages can be received at the current time

    int16_t maxIterations = 50;  //!< the maximum number of iterations allowed for the federate
    int32_t logLevel = 1;  //!< the logging level above which not to log to file
};

}  // namespace helics

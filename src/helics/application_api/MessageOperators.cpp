/*

Copyright © 2017-2018,
Battelle Memorial Institute; Lawrence Livermore National Security, LLC; Alliance for Sustainable Energy, LLC
All rights reserved. See LICENSE file and DISCLAIMER for more details.
*/
#include "MessageOperators.hpp"

namespace helics
{
MessageTimeOperator::MessageTimeOperator (std::function<Time (Time)> userTimeFunction)
    : TimeFunction (std::move (userTimeFunction))
{
}

std::unique_ptr<Message> MessageTimeOperator::process (std::unique_ptr<Message> message)
{
    if (TimeFunction)
    {
        message->time = TimeFunction (message->time);
    }
    return message;
}

void MessageTimeOperator::setTimeFunction (std::function<Time (Time)> userTimeFunction)
{
    TimeFunction = std::move (userTimeFunction);
}

MessageDataOperator::MessageDataOperator (std::function<data_view (data_view)> userDataFunction)
    : dataFunction (std::move (userDataFunction))
{
}

void MessageDataOperator::setDataFunction (std::function<data_view (data_view)> userDataFunction)
{
    dataFunction = std::move (userDataFunction);
}

std::unique_ptr<Message> MessageDataOperator::process (std::unique_ptr<Message> message)
{
    if (dataFunction)
    {
        auto dv = dataFunction (data_view (message->data));
        message->data = dv.to_data_block ();
    }
    return message;
}

MessageDestOperator::MessageDestOperator (
  std::function<std::string (const std::string &, const std::string &)> userDestFunction)
    : DestUpdateFunction (std::move (userDestFunction))
{
}

void MessageDestOperator::setDestFunction (
  std::function<std::string (const std::string &, const std::string &)> userDestFunction)
{
    DestUpdateFunction = std::move (userDestFunction);
}

std::unique_ptr<Message> MessageDestOperator::process (std::unique_ptr<Message> message)
{
    if (DestUpdateFunction)
    {
        message->original_dest = message->dest;
        message->dest = DestUpdateFunction (message->source, message->dest);
    }
    return message;
}

MessageConditionalOperator::MessageConditionalOperator (std::function<bool(const Message *)> userConditionFunction)
    : evalFunction (std::move (userConditionFunction))
{
}

void MessageConditionalOperator::setConditionFunction (std::function<bool(const Message *)> userConditionFunction)
{
    evalFunction = std::move (userConditionFunction);
}

std::unique_ptr<Message> MessageConditionalOperator::process (std::unique_ptr<Message> message)
{
    if (evalFunction)
    {
        if (evalFunction (message.get ()))
        {
            return message;
        }
        return nullptr;
    }
    return message;
}

CloneOperator::CloneOperator (std::function<void(const Message *)> userCloneFunction)
    : evalFunction (std::move (userCloneFunction))
{
}

void CloneOperator::setCloneFunction (std::function<void(const Message *)> userCloneFunction)
{
    evalFunction = std::move (userCloneFunction);
}

std::unique_ptr<Message> CloneOperator::process (std::unique_ptr<Message> message)
{
    if (evalFunction)
    {
        evalFunction (message.get ());
    }
    return message;
}
}  // namespace helics

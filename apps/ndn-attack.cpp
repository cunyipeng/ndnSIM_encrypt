/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2011-2015  Regents of the University of California.
 *
 * This file is part of ndnSIM. See AUTHORS for complete list of ndnSIM authors and
 * contributors.
 *
 * ndnSIM is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * ndnSIM is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * ndnSIM, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 **/

#include "ndn-attack.hpp"
#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"

#include "model/ndn-ns3.hpp"
#include "model/ndn-l3-protocol.hpp"
#include "helper/ndn-fib-helper.hpp"
#include <iostream>
#include <memory>

NS_LOG_COMPONENT_DEFINE("ndn.Attack");

namespace ns3 {
namespace ndn {

NS_OBJECT_ENSURE_REGISTERED(Attack);

TypeId
Attack::GetTypeId(void)
{
  static TypeId tid =
    TypeId("ns3::ndn::Attack")
      .SetGroupName("Ndn")
      .SetParent<App>()
      .AddConstructor<Attack>()
      .AddAttribute("Prefix", "Prefix, for which Attack has the data", StringValue("/"),
                    MakeNameAccessor(&Attack::m_prefix), MakeNameChecker())
      .AddAttribute(
         "Postfix",
         "Postfix that is added to the output data (e.g., for adding Attack-uniqueness)",
         StringValue("/"), MakeNameAccessor(&Attack::m_postfix), MakeNameChecker())
      .AddAttribute("PayloadSize", "Virtual payload size for Content packets", UintegerValue(1024),
                    MakeUintegerAccessor(&Attack::m_virtualPayloadSize),
                    MakeUintegerChecker<uint32_t>())
      .AddAttribute("Freshness", "Freshness of data packets, if 0, then unlimited freshness",
                    TimeValue(Seconds(0)), MakeTimeAccessor(&Attack::m_freshness),
                    MakeTimeChecker())
      .AddAttribute(
         "Signature",
         "Fake signature, 0 valid signature (default), other values application-specific",
         UintegerValue(0), MakeUintegerAccessor(&Attack::m_signature),
         MakeUintegerChecker<uint32_t>())
      .AddAttribute("KeyLocator",
                    "Name to be used for key locator.  If root, then key locator is not used",
                    NameValue(), MakeNameAccessor(&Attack::m_keyLocator), MakeNameChecker());
  return tid;
}

Attack::Attack()
{
  NS_LOG_FUNCTION_NOARGS();
}

// inherited from Application base class.
void
Attack::StartApplication()
{
  NS_LOG_FUNCTION_NOARGS();
  App::StartApplication();

  FibHelper::AddRoute(GetNode(), m_prefix, m_face, 0);
}

void
Attack::StopApplication()
{
  NS_LOG_FUNCTION_NOARGS();

  App::StopApplication();
}
void
Attack::OnData(shared_ptr<Data> data)
{
  if(!m_active)
      return;

  App::OnData(data); // tracing inside

  NS_LOG_FUNCTION(this << data);

  // This could be a problem......
  uint32_t seq = data->getName().at(-1).toSequenceNumber();
  NS_LOG_INFO("< DATA for " << seq);

  //Name dataName(interest->getName());
   //dataName.append("/testApp");
   //dataName.appendVersion();

  //auto data = make_shared<Data>();
  //data->setName(dataName);
  //data->setFreshnessPeriod(::ndn::time::milliseconds(m_freshness.GetMilliSeconds()));

  static const std::string content = "Hello Kitty!";

  data->setContent(reinterpret_cast<const uint8_t*>(content.c_str()), content.size());

  NS_LOG_INFO("node(" << GetNode()->GetId() << ") responding with Data: " << data->getName());
  std::cout<<"node(" << GetNode()->GetId() << ") responding with Data's Content: " << *data << std::endl;
  // to create real wire encoding
  data->wireEncode();

  m_transmittedDatas(data, this, m_face);
  m_appLink->onReceiveData(*data);
}

} // namespace ndn
} // namespace ns3

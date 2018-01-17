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

// ndn-congestion-alt-topo-plugin.cpp

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"

namespace ns3 {

int
main(int argc, char* argv[])
{
  CommandLine cmd;
  cmd.Parse(argc, argv);

  AnnotatedTopologyReader topologyReader("", 1);
  topologyReader.SetFileName("src/ndnSIM/examples/topologies/lte.txt");
  topologyReader.Read();

  // Install NDN stack on all nodes
  ndn::StackHelper ndnHelper;
  ndnHelper.SetOldContentStore("ns3::ndn::cs::Lru", "MaxSize",
                               "1"); // ! Attention ! If set to 0, then MaxSize is infinite
  ndnHelper.InstallAll();

  // Set BestRoute strategy
  ndn::StrategyChoiceHelper::InstallAll("/", "/localhost/nfd/strategy/best-route");

  // Getting containers for the consumer/producer
  Ptr<Node> consumers = Names::Find<Node>("UE");
  Ptr<Node> producers = Names::Find<Node>("Server");
  Ptr<Node> attacks = Names::Find<Node>("PGW");

  if (consumers == 0 || producers == 0) {
    NS_FATAL_ERROR("Error in topology: one nodes UE or Server is missing");
  }
    std::string prefix = "/data/" + Names::FindName(producers);

    /////////////////////////////////////////////////////////////////////////////////
    // install consumer app on consumer node c_i to request data from producer p_i //
    /////////////////////////////////////////////////////////////////////////////////

    ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCbr");
    consumerHelper.SetAttribute("Frequency", StringValue("10")); // 100 interests a second

    consumerHelper.SetPrefix(prefix);
    consumerHelper.SetAttribute("Signature", StringValue("1"));
    ApplicationContainer consumer = consumerHelper.Install(consumers);
    consumer.Start(Seconds(0));     
    
    consumer.Stop(Seconds(19)); 

    /////////////////////////////////////////////////////////////////////////////////
    // install attack app on attack node c_i to modify data from producer p_i //
    /////////////////////////////////////////////////////////////////////////////////
       
    //ndn::AppHelper attackHelper("ns3::ndn::Attack");
    //consumerHelper.SetAttribute("Frequency", StringValue("10")); // 100 interests a second

    //attackHelper.SetPrefix(prefix);
    //attackHelper.SetAttribute("Signature", StringValue("1"));
    //attackHelper.SetAttribute("PayloadSize", StringValue("1024"));
    //ApplicationContainer attack = attackHelper.Install(attacks);
    //consumer.Start(Seconds(0));     
    
    //consumer.Stop(Seconds(19));  


    ///////////////////////////////////////////////
    // install producer app on producer node p_i //
    ///////////////////////////////////////////////

    ndn::AppHelper producerHelper("ns3::ndn::Producer");
    producerHelper.SetAttribute("PayloadSize", StringValue("1024"));
    producerHelper.SetAttribute("Signature", StringValue("1"));

    //install producer that will satisfy Interests in /dst1 namespace
    producerHelper.SetPrefix(prefix);
    ApplicationContainer producer = producerHelper.Install(producers);
    // when Start/Stop time is not specified, the application is running throughout the simulation

  // Manually configure FIB routes
  ndn::FibHelper::AddRoute("UE", "/data", "eNB1", 1); 
  ndn::FibHelper::AddRoute("eNB1", "/data", "eNB3", 2); 
  ndn::FibHelper::AddRoute("eNB2", "/data", "eNB3", 1); 
  ndn::FibHelper::AddRoute("eNB1", "/data", "eNB2", 1); 

  ndn::FibHelper::AddRoute("eNB1", "/data", "MME", 1); 
  ndn::FibHelper::AddRoute("eNB1", "/data", "SGW", 1); 
  ndn::FibHelper::AddRoute("eNB2", "/data", "MME", 1); 
  ndn::FibHelper::AddRoute("eNB2", "/data", "SGW", 1); 
  ndn::FibHelper::AddRoute("eNB3", "/data", "MME", 1); 
  ndn::FibHelper::AddRoute("eNB3", "/data", "SGW", 2); 

  ndn::FibHelper::AddRoute("MME", "/data", "SGW", 1); 
  ndn::FibHelper::AddRoute("SGW", "/data", "PGW", 1); 
  ndn::FibHelper::AddRoute("PGW", "/data", "Server", 1); 

  // Schedule simulation time and run the simulation
  Simulator::Stop(Seconds(20.0));
  Simulator::Run();
  Simulator::Destroy();

  return 0;
}

} // namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}

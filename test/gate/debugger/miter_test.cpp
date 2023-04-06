//===----------------------------------------------------------------------===//
//
// Part of the Utopia EDA Project, under the Apache License v2.0
// SPDX-License-Identifier: Apache-2.0
// Copyright 2023 ISP RAS (http://www.ispras.ru)
//
//===----------------------------------------------------------------------===//

#include "gate/debugger/miter.h"
#include "gate/model/gnet_test.h"

#include "gtest/gtest.h"

using namespace eda::gate::debugger;
using namespace eda::gate::model;

TEST(MiterTest, MiterStructureTest) {
  auto net = new GNet();
  SignalList inps;
  for (int i = 0; i<10;i++){
    GateId z = net->addIn();
    inps.push_back(Signal::always(z));
  }
  GateId y = net->addGate(GateSymbol::AND, inps);
  GateId outId = net->addOut(y);
  SignalList inps1;
  for (int i = 0; i<10;i++){
    GateId z = net->addIn();
    inps1.push_back(Signal::always(z));
  }
  GateId w = net->addGate(GateSymbol::NOR, inps1);
  net->addOut(w);
  w = net->addGate(GateSymbol::OR, inps);
  for(int i = 0; i<10;i++){
    outId = net->addOut(w);
  }
  net->addOut(w);
  std::unordered_map<Gate::Id, Gate::Id> testMap = {};
  auto netCloned = net->clone(testMap);
  
  GateBinding ibind, obind, tbind;

  // Input-to-input correspondence.
  for (auto oldSourceLink : net->sourceLinks()) {
    auto newSourceId = testMap[oldSourceLink.target];
    ibind.insert({oldSourceLink, Gate::Link(newSourceId)});
  }

  // Output-to-output correspondence.
  for (auto oldTargetLink : net->targetLinks()) {
    auto newTargetId = testMap[oldTargetLink.source];
    obind.insert({oldTargetLink, Gate::Link(newTargetId)});
  }

  // Trigger-to-trigger correspondence.
  for (auto oldTriggerId : net->triggers()) {
    auto newTriggerId = testMap[oldTriggerId];
    tbind.insert({Gate::Link(oldTriggerId), Gate::Link(newTriggerId)});
  }

  Checker::Hints hints;
  hints.sourceBinding  = std::make_shared<GateBinding>(std::move(ibind));
  hints.targetBinding  = std::make_shared<GateBinding>(std::move(obind));
  hints.triggerBinding = std::make_shared<GateBinding>(std::move(tbind));
  
  GNet* mit = miter(net, netCloned, hints);
  EXPECT_TRUE(mit->nTargetLinks() == 1);
  EXPECT_TRUE(mit->nSourceLinks() == netCloned->nSourceLinks());
}


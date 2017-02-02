/*
 * Copyright (c) 2002 by Cisco Systems, Inc.
 * All rights reserved.
 */

#include <CxManPort.h>


CxManPort::CxManPort() {
    
    pimPhyport_ = 0;
    lastGotStatsMicroTime_ = 0;
    enabled_ = false;
    halCx_ = 0;
    portId_ = HalStubPortIdNull;
    ledId_ = HalStubLedIdNull;
    phyId_ = HalStubPhyIdNull;
    runMode_ = RunModeReboot;
    debugLevel_ = 1;
    cxMan_ = 0;
    Store * store = ThisStore( this );

    halEthMiiDevice_ = new( store ) CxManPortHalEthMiiDevice( this );
    ethMiiPhyMan_ = new( store ) EthMiiPhyMan;
    pimEthAutoNegotiator_ = new( store ) PimEthAutoNegotiator;
    ethMiiPhyMan_->halEthMiiDevice( halEthMiiDevice_ );
    // new dpmstatemachine
    sm_ = new( store ) StateMachine();
    pimEthAutoNegotiator_->dpmStateMachine( sm_ );

    State * linkDown = new State( 0, "LinkDown" );

    State * linkUp = new State( 1, "LinkUp" );

    State * standBy = new State( 2, "StandBy" );
    
    sm_->newState( linkDown );
    sm_->newState( linkUp );
    sm_->newState( standBy );
    
    Transition * linkDownToLinkUp = new Transition( linkDown, linkUp, SmEventLinkUp, f1 );

    Transition * linkUptoLinkDown = new Transition( linkUp, linkDown, SmEventLinkDown, f2 );

    Transition * linkDownToStandBy = new Transition( linkDown, standBy, SmEventStandBy, f3 );

    Transition * standByToLinkDown = new Transition( standBy, linkDown, SmEventLinkDown, f4 );

    Transition * linkUpToStandBy = new Transition( linkUp, standBy, SmEventStandBy, f5 );

    Transition * standByToLinkUp = new Transition( standBy, linkUp, SmEventLinkUp, f6 );

    sm->newTransition( linkDown, linkDownToStandBy );
    sm->newTransition( linkDown, linkDownToLinkUp );
    
    sm->newTransition( standBy, standByToLinkDown );
    sm->newTransition( standBy, standByToLinkUp );
    
    sm->newTransition( linkUp, linkUpToLinkDown );
    sm->newTransition( linkUp, linkUpToStandBy );    


    pimEthAutoNegotiator_->ethAutoNegotiator(
        ethMiiPhyMan_->ethAutoNegotiator() );
    ethMiiPhyMan_->clock( DefaultClock() );
    
    lastFastEthernetToGigabitFifoHeadPtr_ = HalCxHeadPtr( 0 );
    lastFastEthernetToGigabitFifoTailPtr_ = HalCxTailPtr( 0 );
    stuckUplinkFifoCountSinceLastClear_ = 0;
    totalStuckUplinkFifoCount_ = 0;
    lastFastEthernetToGigabitDiagBlockReadTime_ = Time( 0 );
}

void
PimEthAutoNegotiator::runDpmReview() {
    dpmStateMachine_->run();
}


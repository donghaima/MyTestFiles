/*
 * Copyright (c) 2002 by Cisco Systems, Inc.
 * All rights reserved.
 */

#include <SmEvent.h>
#include <State.h>
#include <Transition.h>
#include <StateMachine.h>

class LinkDownToLinkUpNotify : public SmEventNotify {
  public:
    void onNotify() {
        doA();
    }    
}

class LinkDownToStandByNotify : public SmEventNotify {
  public:
    void onNotify() {
        doB();
    }    
}

class LinkUpToLinkDownNotify : public SmEventNotify {
  public:
    void onNotify() {
        doC();
    }    
}

class LinkUpToStandByNotify : public SmEventNotify {
  public:
    void onNotify() {
        doD();
    }    
}

class StandByToLinkDownNotify : public SmEventNotify {
  public:
    void onNotify() {
        doE();
    }    
}

class StandByToLinkUpNotify : public SmEventNotify {
  public:
    void onNotify() {
        doF();
    }    
}

// constructor
smExample() {
    StateMachine * sm = new StateMachine();

    State * linkDown = new State( 0, "LinkDown" );

    State * linkUp = new State( 1, "LinkUp" );

    State * standBy = new State( 2, "StandBy" );
    
    sm->newState( linkDown );
    sm->newState( linkUp );
    sm->newState( standBy );
    
    Transition * linkDownToLinkUp = new Transition();
    linkDownToLinkUp->smEvent( SmEventLinkUp );
    linkDownToLinkUp->currentState( linkDown );
    linkDownToLinkUp->nextState( linkUp );
    SmEventNotify * linkDownToLinkUpNotify = new LinkDownToLinkUpNotify();
    linkDownToLinkUp->onEventNotify( linkDownToLinkUpNotify );

    Transition * linkUptoLinkDown = new Transition();
    linkUptoLinkDown->smEvent( SmEventLinkDown );
    linkUptoLinkDown->currentState( linkUp );
    linkUptoLinkDown->nextState( linkDown );
    SmEventNotify * linkUpToLinkDownNotify = new LinkUpToLinkDownNotify();
    linkUpToLinkDown->onEventNotify( linkUpToLinkDownNotify );


    Transition * linkDownToStandBy = new Transition();
    linkDownToStandBy->smEvent( SmEventStandBy );
    linkDownToStandBy->currentState( linkDown );
    linkDownToStandBy->nextState( standBy );
    SmEventNotify * linkDownToStandByNotify = new LinkDownToStandByNotify();
    linkDownToStandBy->onEventNotify( linkDownToStandByNotify );

    Transition * standByToLinkDown = new Transition();
    standByToLinkDown->smEvent( SmEventLinkDown );
    standByToLinkDown->currentState( standBy );
    standByToLinkDown->nextState( linkDown );
    SmEventNotify * standByToLinkDownNotify = new StandByToLinkDownNotify();
    standByToLinkDown->onEventNotify( standByToLinkDownNotify );

    Transition * linkUpToStandBy = new Transition();
    linkUpToStandBy->smEvent( SmEventStandBy );
    linkUpToStandBy->currentState( linkUp );
    linkUpToStandBy->nextState( standBy );
    SmEventNotify * linkUpToStandByNotify = new LinkUpToStandByNotify();
    linkUpToStandBy->onEventNotify( linkUpToStandByNotify );

    Transition * standByToLinkUp = new Transition();
    standByToLinkUp->smEvent( SmEventLinkUp );
    standByToLinkUp->currentState( standBy );
    standByToLinkUp->nextState( linkUp );
    SmEventNotify * standByToLinkUpNotify = new StandByToLinkUpNotify();
    standByToLinkUp->onEventNotify( standByToLinkUpNotify );

    sm->newTransition( linkDown, linkDownToStandBy );
    sm->newTransition( linkDown, linkDownToLinkUp );
    
    sm->newTransition( standBy, standByToLinkDown );
    sm->newTransition( standBy, standByToLinkUp );
    
    sm->newTransition( linkUp, linkUpToLinkDown );
    sm->newTransition( linkUp, linkUpToStandBy );    
}

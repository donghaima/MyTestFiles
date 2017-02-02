/*
 * Copyright (c) 2002 by Cisco Systems, Inc.
 * All rights reserved.
 */

#include <SmEvent.h>
#include <State.h>
#include <Transition.h>
#include <StateMachine.h>


// constructor
smExample() {

    StateMachine * sm = new StateMachine();
    
    void f1();
    void f2();
    void f3();
    void f4();
    void f5();
    void f6();
    
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
    linkDownToLinkUp->onEventNotify( this, f1 );

    Transition * linkUptoLinkDown = new Transition();
    linkUptoLinkDown->smEvent( SmEventLinkDown );
    linkUptoLinkDown->currentState( linkUp );
    linkUptoLinkDown->nextState( linkDown );
    SmEventNotify * linkUpToLinkDownNotify = new LinkUpToLinkDownNotify();
    linkUpToLinkDown->onEventNotify( this, f2 );


    Transition * linkDownToStandBy = new Transition();
    linkDownToStandBy->smEvent( SmEventStandBy );
    linkDownToStandBy->currentState( linkDown );
    linkDownToStandBy->nextState( standBy );
    SmEventNotify * linkDownToStandByNotify = new LinkDownToStandByNotify();
    linkDownToStandBy->onEventNotify( this, f3 );

    Transition * standByToLinkDown = new Transition();
    standByToLinkDown->smEvent( SmEventLinkDown );
    standByToLinkDown->currentState( standBy );
    standByToLinkDown->nextState( linkDown );
    SmEventNotify * standByToLinkDownNotify = new StandByToLinkDownNotify();
    standByToLinkDown->onEventNotify( this, f4 );

    Transition * linkUpToStandBy = new Transition();
    linkUpToStandBy->smEvent( SmEventStandBy );
    linkUpToStandBy->currentState( linkUp );
    linkUpToStandBy->nextState( standBy );
    SmEventNotify * linkUpToStandByNotify = new LinkUpToStandByNotify();
    linkUpToStandBy->onEventNotify( this, f5 );

    Transition * standByToLinkUp = new Transition();
    standByToLinkUp->smEvent( SmEventLinkUp );
    standByToLinkUp->currentState( standBy );
    standByToLinkUp->nextState( linkUp );
    SmEventNotify * standByToLinkUpNotify = new StandByToLinkUpNotify();
    standByToLinkUp->onEventNotify( this, f6 );

    sm->newTransition( linkDown, linkDownToStandBy );
    sm->newTransition( linkDown, linkDownToLinkUp );
    
    sm->newTransition( standBy, standByToLinkDown );
    sm->newTransition( standBy, standByToLinkUp );
    
    sm->newTransition( linkUp, linkUpToLinkDown );
    sm->newTransition( linkUp, linkUpToStandBy );    
}

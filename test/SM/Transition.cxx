/*
 * Copyright (c) 2002 by Cisco Systems, Inc.
 * All rights reserved.
 */

#include <Transition.h>

Transition::Transition() {
    event_ = 0;
    eventNotify_ = 0;
    nextState_ = 0;  
}
    
void 
Transition::smEvent( SmEvent e ) {
    event_ = e;
}

SmEvent 
Transition::smEvent() {
    return event_;
}

void 
Transition::onEventNotify( SmEventNotify * notify ) {
    assert( notify );
    eventNotify_ = notify;
}

SmEventNotify *
Transition::onEventNotify() {
    return eventNotify_;
}

void 
Transition::nextState( State * s ) {
    assert( s );
    nextState_ = s;
}

State 
Transition::nextState() {
    return nextState_;
}

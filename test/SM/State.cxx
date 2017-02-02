/*
 * Copyright (c) 2002 by Cisco Systems, Inc.
 * All rights reserved.
 */

#include <State.h>

State::State() {
    enterNotify_ = 0;
    exitNotify_ = 0;
    stateId_ = 0;
    name_ = 0;
}

void 
State::onEnterNotify( SmEventNotify * notify ) {
    assert( notify );
    enterNotify_ = notify;
}

SmEventNotify * 
State::onEnterNotify() {
    return enterNotify_;
}

void 
State::onExitNotify( SmEventNotify * notify ) {
    assert( notify );
    exitNotify_ = notify;
}

SmEventNotify * 
State::onExitNotify() {
    return exitNotify_;
}

StateId 
State::id() {
    return stateId_;
}

void 
State::id( StateId id ) {
    stateId_ = id;
}

void 
State::name( Name name ) {
    name_ = name;
}

Name 
State::name() {
    return name_;
}

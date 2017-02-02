/*
 * Copyright (c) 2002 by Cisco Systems, Inc.
 * All rights reserved.
 */

#include <SmEvent.h>

SmEvent::SmEvent() {
    name_ = "uninit";
    smEventId_ = 0;
    persistent_ = false;
    optional_ = false;
    atomicNotify_ = false;
};

void 
SmEvent::name( Name name ) {
    name_ = name;
}

Name 
SmEvent::name() {
    return name_;
}

void 
SmEvent::id( SmEventId id ) {
    smEventId_ = id;
}

SmEventId 
SmEvent::id() {
    return smEventId_;
}

void 
SmEvent::persistent( bool b ) {
    persistent_ = b;
}

bool 
SmEvent::persistent() {
    return persistent_;
}

void 
SmEvent::optional( bool b ) {
    optional_ = b;
}

bool 
SmEvent::optional() {
    return optional_;
}
 
void 
SmEvent::atomicNotify( bool b ) {
    atomicNotify_ = b;
}

bool 
SmEvent::atomicNotify() {
    return atomicNotify_;
}

bool 
SmEvent::operator==( SmEvent const & x ) {
    return( name_ == x.name_ && smEventId_ == x.smEventId_ &&
            persistent_ == x.persistent_ && optional_ == x.optional_ &&
            atomicNotify_ == x.atomicNotify_ );
}

bool 
SmEvent::operator!=( SmEvent const & x ) {
    return( !( this == x ) );
}
 
void 
SmEvent::operator=( SmEvent const & x ) {
    name_ = x.name_; 
    smEventId_ = x.smEventId_;
    persistent_ = x.persistent_;
    optional_ = x.optional_;
    atomicNotify_ = x.atomicNotify_;
}

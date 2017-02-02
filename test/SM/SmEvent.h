/*
 * Copyright (c) 2002 by Cisco Systems, Inc.
 * All rights reserved.
 */
#ifndef SMEVENT_H
#define SMEVENT_H

typedef int SmEventId;
typedef int Name;

// SmEvent represents an event in the state machine.  It doesn't have any 
// state associated with it.  We just need a global id that can be used to
// differentiate it from events across other state machines.  This is needed
// because state machines could be merged together.
class SmEvent {
  public:
    SmEvent();

    void name( Name );
    Name name();

    // This is needed so that the event id can be unique across
    // all state machines and erraneous events thrown on a state
    // machine can be caught.
    void id( SmEventId );
    SmEventId id();

    // The persistent attribute is set on an event if it should be thrown
    // automatically after making appropriate notifications.  The event will
    // therefore be thrown until someone makes it non-persistent (after which
    // it will not be thrown at the state machine).
    void persistent( bool );
    bool persistent();

    // Unless the optional attribute is set, receipt of an "unintended" event 
    // will trigger an error.
    void optional( bool );
    bool optional();

    // Unless atomicNotify is set to true, the state machine could yield the 
    // CPU before a notification on behalf of this event is made.
    void atomicNotify( bool );
    bool atomicNotify();

    bool operator==( SmEvent const & );
    bool operator!=( SmEvent const & );
    void operator=( SmEvent const & );

  private:
    Name name_;
    SmEventId smEventId_;
    bool persistent_;
    bool optional_;
    bool atomicNotify_;

};

#endif /* SMEVENT_H */

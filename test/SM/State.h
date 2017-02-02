/*
 * Copyright (c) 2002 by Cisco Systems, Inc.
 * All rights reserved.
 */
#ifndef STATE_H
#define STATE_H

#include <SmEventNotify.h>
typedef int StateId;

// Representation of State.  It has two notfiees hanging off of it.  One 
// for enter and other for exit.
class State {
  public:
    State();

    // Every state could be setup with enter and exit notifiees.  The enter 
    // notifiee will be notified whenever you enter this state from a 
    // different state.   The exit notifee will be notified when the state 
    // machine moves from this state to a different state.
    void onEnterNotify( SmEventNotify * );
    SmEventNotify * onEnterNotify();

    void onExitNotify( SmEventNotify * );
    SmEventNotify * onExitNotify();

    // Assign an id to the state.  This is global across all state machines.
    StateId id();
    void id( StateId );

    // Let's give a name to this state.  Useful for debugging.
    void name( Name );
    Name name();

    int numTransitions();
    void newTransition( Transition * );
    void deleteTransition( Transition * );

  private:
    SmEventNotify * enterNotify_;
    SmEventNotify * exitNotify_;
    StateId stateId_;
    Name name_;
    Transition * transitions_[256];
    int numTransitions_;
};

#endif /* STATE_H */

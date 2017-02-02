/*
 * Copyright (c) 2002 by Cisco Systems, Inc.
 * All rights reserved.
 */
#ifndef TRANSITION_H
#define TRANSITION_H

// A Transition pulls the event, notifiee and the next state together into
// one single class.  Transitions are passed to the state machine when it's
// being setup.
class Transition {
  public:
    Transition();
    
    // The event based on which this transition is taken.
    void smEvent( SmEvent );
    SmEvent smEvent();

    // The notifiee to be notified when this transition is taken.  Note
    // that as of now, this notifee will be notified before the exit notifee
    // is notified.
    void onEventNotify( SmEventNotify * );
    SmEventNotify onEventNotify();

    void currentState( State * );
    State * currentState();
    // The state to go for after the transition is made.
    void nextState( State * );
    State * nextState();

  private:
    SmEvent event_;
    SmEventNotify * eventNotify_;
    State * currentState_;
    State * nextState_;
};

#endif /* TRANSITION_H */

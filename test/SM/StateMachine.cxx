/*
 * Copyright (c) 2002 by Cisco Systems, Inc.
 * All rights reserved.
 */

#include <StateMachine.h>

    StateMachine();

    // Infrastructure needed to setup the state machine.  People will
    // use this part of the interface to setup the state machine.
    void newState( State );
    void newEvent( SmEvent );
    void newTransition( State, Transition );

    // Restructure the state machine for some specific task.

    // Number of transitions out of a given state.
    u_int numTransitions( State );

    // Nth transition out of a given state.
    Transition nthTransition( State, u_int );

    // Returns a transition if it exists between the firstState and 
    // secondState.  Otherwise, it returns a null transition.  This
    // is particularly useful during static hook-up of new states/
    // transitions into an existing state machine.  This assumes that
    // the person doing the hooks has knowledge about the state machine
    // to which the modifications are being made.
    Transition transition( State firstState, State secondState );

    void deleteState( State );
    void deleteEvent( SmEvent );
    void deleteTransition( State, Transition );

    // The start state should be one among the set of states added via
    // newState.
    void startState( State );

    // Control the execution of state machine.
    void run( bool );

    // the operating state of the state machine - running, suspended, 
    // not started?
    SmOpState opState();

    // Throw an event at the state machine.
    void smEvent( SmEvent );

    // Find out the current state of state machine.
    State currentSmState();

    // Number of pending events.
    u_int pendingEvents();

    // return the nth event pending to be processesed - 0th pending event is
    // the next one to be processed.
    SmEvent nthPendingSmEvent( u_int );

    void name( Name );
    Name name();

    // Job control stuff goes here.
    // XXX pjonnala

    // Debugging stuff.
    void currentSmState( State );
    void nthPendingSmEvent( u_int, SmEvent );

    void debugLevel( u_int );
    u_int debugLevel();

    // History - Queue of transitions that have been taken recently.
    State initialState();
    u_int numTransitions();
    Transition nthTransition( u_int );
    u_int nthTransitionCount( u_int );
/*
 * Copyright (c) 2002 by Cisco Systems, Inc.
 * All rights reserved.
 */
#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <gs/SingleLinkedListUnsafe.h>
#include <gs/HashTab.h>

class TransitionEntryKey {
  public:

    TransitionEntryKey( State * state, Event event ) {
        state_ = state;
        event_ = event;
    }

    ~TransitionEntryKey() {}

    bool operator==( TransitionEntryKey const & that ) const {
        return ( state_ == that.state_ && event == that.event_ );
    }

    u_int hashFunction() const {
        u_int val = 0;
        assert( state_ );
        val = ( state_->id() + event.id() % MaxEventPerState );
        return val;
    }   

  private:
    State * state_;
    Event event_;
}

class TransitionEntry {
  public:
  private:
    friend class TsmMemMan<TransitionEntry>;
    friend class HashTab<TransitionEntry, TransitionEntryKey>;

    TransitionEntry( Transition * transtion ) {
        transition_ = transition;
    }
    ~TransitionEntry() {}
    
    Transition * transition() {
        return transition_;
    }

    // HashTab support
    static u_int hashFunction( TransitionEntryKey const & key ) {
        return key.hashFunction();
    }
    bool hashMatch( TransitionEntryKey const & key ) {
        return ( key == key_ );
    }
    TransitionEntryKey const & hashKey() { return key_; }
    TransitionEntry * hashNext_;

    // TSM support
    TransitionEntry * nextFreeObj() { return tsmNextFree_; }
    void nextFreeObj( TransitionEntry * nf ) { tsmNextFree_ = nf; }
    
    TransitionEntryKey key_;
    Transition * transition_;
}

// StateMachine encapsulates states, events and transitions.  The state 
// machine takes the responsibility of handling events thrown at it, make
// appropriate notifications and move the current state along.  While
// running, it stores information that will be helpful when debugging 
// problems.
class StateMachine {
  public:
    StateMachine();

    // Infrastructure needed to setup the state machine.  People will
    // use this part of the interface to setup the state machine.
    void newState( State );
    // Try to implement this by LinkList????
    // SinglyLinkedListUnsafe ??


    void newEvent( SmEvent );
    // Can we implement this by an array like event[1024]??
    // Once an event is thrown, can we delete it? 
    // If it's no, then we can just use array to implement binarry linklist.

    void newTransition( State, Transition );
    // We can implement this by an LinkList????
    // So each state will point to several transition??? hashed by event????

    // Restructure the state machine for some specific task.

    // Number of transitions out of a given state.
    u_int numTransitions( State );
    // Can we keep this infomation in State???

    // Nth transition out of a given state.
    Transition nthTransition( State, u_int );

    // Returns a transition if it exists between the firstState and 
    // secondState.  Otherwise, it returns a null transition.  This
    // is particularly useful during static hook-up of new states/
    // transitions into an existing state machine.  This assumes that
    // the person doing the hooks has knowledge about the state machine
    // to which the modifications are being made.
    Transition transition( State firstState, State secondState );
    // So we need to go throught all transition from a state:)

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
    // if it's in array, or linklist, we can easily get this.

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
    // We need an array to maintain this????
    Transition nthTransition( u_int );
    u_int nthTransitionCount( u_int );

  private:
    // create a type for list of State.
    typedef SinglyLinkedListNode< State * > StateListNode;
    typedef SinglyLinkedListUnsafe< State * > StateList;
    typedef SinglyLinkedListUnsafeIterator< State * > StateListIterator;
    TsmMemMan< StateListNode > * Manager( StateListNode const * );
    
    SmEvent events_[ 256 ];
    
    // implementation for hashtable, entry is transition*
    // key is State* and event
    typedef HashTab<TransitionEntry, TransitionEntryKey> 
        TransitionEntryHashTab;
    static u_int const TransitionEntryHashTabBuckets = 
        MaxStates * MaxEventPerState;
    TransitionEntry *
        transitionEntryHashTabBucketArray_[ TransitionEntryHashTabBuckets ];
    TransitionEntryHashTab transitonEntryHashTab_;

    //
    State * startState_;
    State * currentState_;

    // control the running of the statemachine
    bool run_;
    
    // the operating mode of the statemachine
    enum SmOpState{
        SmOpStateNull,
        SmOpStateReboot,
        SmOpStateReset,
        SmOpStateFrozen,
        SmOpStateOperating
    };
    SmOpState opState_;

    u_int numPendingEvents_;
    SmEvents pendingEvents[256];

    Name name_;
    
    // Debugging stuff
    u_int debugLevel_;

    u_int numTransitions_;
    Transition * transitions_[ 256 ];
    Transition * nthTransition( u_int );   
    void dumpStateMachine();
};

#endif /* STATEMACHINE_H */

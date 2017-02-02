// Please refer to the EDCS-198877
Interface is as following.

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

// The SmEventNotify class should be implemented by parties interested
// in getting notifications for entry, exit, or transitions.  Currently,
// we will not have a proxy to support multiple notifications.  It will
// be eventually added when we find a need for that.
class SmEventNotify {
  public:
    // We always pass the state machine to notifiees so that they can 
    // get any information that they might need.
    virtual void onNotify( StateMachine * ) = 0;
    virtual Name name() = 0;
};


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


!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
Open issues:
1:
Do we really need persistent, optional, and atomic in SmEvent class?

2:
How do we implement the transition in state machine.
I think it's an attribute in State class. From State, you can easily get
transition from event. And it's easy to think of the object model.

But Prem think it's better let state machine has all the information.
And keep them in the HashTable. So currently I have both interface in
above classes.

3:
Do we really need this? How often we can use this?
Since we can't schedule a job reviewer for every port, that will be a cpu hog.
So the reality will be like current code, we have runReview in each state 
machine, it's called by high level runReview().


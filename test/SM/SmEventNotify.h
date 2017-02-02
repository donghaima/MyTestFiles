/*
 * Copyright (c) 2002 by Cisco Systems, Inc.
 * All rights reserved.
 */
#ifndef SMEVENTNOTIFY_H
#define SMEVENTNOTIFY_H

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

#endif /* SMEVENTNOTIFY_H */

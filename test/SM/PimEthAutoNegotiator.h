/*
 * Copyright (c) 1998, 2000, 2001, 2002 by Cisco Systems, Inc.
 * All rights reserved.
 */
#ifndef PIM_PIMETHAUTONEGOTIATOR_H
#define PIM_PIMETHAUTONEGOTIATOR_H

#include <pim/Types.h>
#include <eth/Types.h>
#include <event/Types.h>
#include <gs/Types.h>

// This class takes care of all physical-layer and inline power management
// for a PimPhyport that is an ethernet port.  If you are a hardware driver
// for an ethernet port (such as ScxGigaportMan or a CxManPort or a 
// NicerManPort), you can instantiate me, connect me to the PimPhyport that
// you want managed, and give me an EthAutoNegotiator and an 
// EthDtePowerController(possibly) abstract interface that will be used
// to control the physical link configuration and inline power respectively.
// PimEthAutoNegotiator takes care of keeping the EthDtePowerController/
// EthAutoNegotiator and Pim state consistent (and safe in case of ports
// that support inline power).  See src/lib/galman/GalSantanaMan.cxx for
// an example of EthDtePowerController and src/lib/ethmii/EthMiiPhyMan.cxx
// for an example of EthAutoNegotiator.

// Some abbreviations:
// DPM -- DTE power via MDI
// DTE -- Data Terminal Equipment
// MDI -- Media Dependent interface

// High Level overview of what PimEthAutoNegotiator does:
// PimEthAutoNegotiator runs two state machines - DPM and non-DPM in case
// of ports that support inline power and non-DPM only in case of ports
// that don't support inline power.  The function runDpmStateMachine runs
// the dpm state machine which takes care of delayed ESMP to find out
// the results of discovery, loopbacks which behave like telecasters,
// delayed NMP responses, etc. Look at ENG-67481 for a detailed explanation
// of the DPM state machine.

// runDpmStateMachine should never be run on ports
// - that don't support inline power, 
// - where inline power configuration has been turned off, and
// - that are disabled.
// runDpmStateMachine runs in all other cases and uses
// runNonDpmReview eventually once the link comes up.

class PimEthAutoNegotiator {
    
  public:
    
    PimEthAutoNegotiator();
    ~PimEthAutoNegotiator();

    // Tell me which PimPhyport holds management and hardware state for the
    // Ethernet plug hole you want managed.  If 0, then I presume the link
    // should be disabled.
    void pimPhyport( PimPhyport * );
    PimPhyport * pimPhyport();

    // Give me an autonegotiator interface that is wired to the Ethernet plug
    // hole you want managed.
    void ethAutoNegotiator( EthAutoNegotiator * );
    EthAutoNegotiator * ethAutoNegotiator();

    // EthDtePowerController is needed for ports that support inline power.
    // It is invalid to have the PimPhyport tell that it supports inline
    // power, but PimEthAutoNegotiator not have an EthDtePowerController.
    void ethDtePowerController( EthDtePowerController * );
    EthDtePowerController * ethDtePowerController();

    // You should call this periodically to make sure I stay synced up.
    // The more often you call this, the lower the latency between PimPhyport
    // changes and EthAutoNegotiator changes.  100 to 200 milliseconds seems
    // like a reasonable estimate.
    void runReview();

    u_int32 dtePowerOutOfControlCounts();

    int debugLevel();
    void debugLevel( int );

    void dump( Stream *, DumpLevel );


    // Give me a device that I use to manipulate the Ethernet link at the
    // physical layer.
    void halEthMiiDevice( HalEthMiiDevice * );
    HalEthMiiDevice * halEthMiiDevice();

    // Tell me what type of mii phy I am supporting
    void ethMiiPhyType( EthMiiPhyType );
    EthMiiPhyType ethMiiPhyType();


  private:

    PimPhyport * pimPhyport_;

    // Non-DPM members.
    EthAutoNegotiator * ethAutoNegotiator_;

    // Members for dpm state machine.
    EthDtePowerController * ethDtePowerController_;

    Microseconds powerOnTime_;
    Microseconds timeDiscoveryTurnedOnAfterDeny_;
    Microseconds timeFirstDtePowerOutOfControl_;

    bool notifiedManagerOfDpmStatus_;
    bool notifiedManagerOfPowerOnAndPowerStatusNotOk_;

    bool hardwareErrorMessageLogged_;

    EthLineSpeedSet oldLineSpeedSet_;
    EthDuplexSet oldDuplex_;
    bool oldAutoNegotiationEnable_;

    Clock * clock_;

    EthConnectorType ethConnectorType( PimConnectorType );

    int debugLevel_;

    enum {
        // 2.5 seconds.
        MaxWaitTimeForManagersDecision = 2500 * 1000,
        
        // 100 milli seconds.
        MaxWaitTimeForPowerOnPropagation = 100 * 1000,

        // 2 milli seconds,
        MaxWaitTimeForPowerDownPropagation = 2 * 1000,

        // 3 seconds.
        MaxTimeForDiscovery = 3 * 1000 * 1000,

        // 8 seconds.
        MaxTimeToLinkUpAfterPowerUp = 8 * 1000 * 1000,
        
        // 500 milli seconds.
        MaxWaitTimeForPowerAvailability = 500 * 1000, 

        MaxNumberOfDistinctStatesStoredForDebug = 16,
    };

    static Microseconds const DelayCheckTimeWhenPowerDenied = 3 * 1000 * 1000;

    // PimDpmState is a state in the PerPort state machine
    // for controlling inline power on DPM capable ports.
    // Look at the Software Design Spec for Veo to see what
    // the following states mean. -pjonnala

    enum PimDpmState {
        PimDpmStateNull,
        PimDpmStateOff,
        PimDpmStateDetecting,
        PimDpmStateWaitForLinkup,
        PimDpmStateLinkup,
    };

    PimDpmState dpmState_;

    void runNonDpmReview();
    void runDpmReview();

    // Helper functions
    void runCiscoDiscovery( PimPhyport *, bool * );
    void restartCiscoDiscovery( PimPhyport * );
    void dpmDeviceDiscoveryMode( PimPhyport *, bool );
    void dpmLinkupTimerStart();
    bool dpmLinkupTimerTimeOut();
    void dpmResetState();
    Microseconds dpmWaitLinkupTimer_;

    void dpmDelayCheckTimerStart();
    bool dpmDelayCheckTimerTimeOut();
    Microseconds dpmDelayCheckTimer_;
    bool dpmDiscoveryModeConfigured_;

    void applyUsersConfig();
    void updateResultsFromAutoNegotiation();

    bool configChangedSinceLastApplied();
    void recordOldConfiguration();

    void dtePowerOutOfControl( PimDpmState );

    u_int32 dtePowerOutOfControlCounts_;
    u_int32 dtePowerOutOfControlSampleCounts_;
    u_int32 noResponseInExpectedTime_;
    u_int32 normalShutOffsByHardware_;
    u_int32 whileMonitoring_;
    u_int32 timerExpiry_;
    u_int32 possibleLoopbacks_;
    u_int32 powerDenyAfterBringup_;
    u_int32 powerDenyAfterPowerup_;
    u_int32 powerDenyBeforePowerup_;
    u_int32 powerOnNotPossible_;
    
    // For port debouncing. We save when we detect the first link down.
    // If the link continues to stay down for at least the port debounce time,
    // then we will report the link down. This means we ignore the link down
    // until it has been down for at least the port debounce time.
    Microseconds firstLinkDownTime_;

    // How do I manipulate the outside world.
    HalEthMiiDevice * halEthMiiDevice_;
    Clock * clock_;

    // Local state.
    enum State {
        StateNull,

        // First configure the phy to its default state in case diagnostics
        // mucked with the values, or there is some special initialization to
        // be done, e.g. hardware bug fix.  This initialization is intended for
        // after a transition from RunModeReset to RunModeFrozen
        StateWaitPhyInit,

        // We have pushed out a request to take the link down, and are waiting
        // for the parameters to all be "pushed" to the MII device.  When this
        // is complete, we will configure autonegotiation.
        StateWaitLinkDown,
        
        // Autonegotiation has been configured, but we are waiting for the
        // parameters to all be "pushed" to the MII device.  When this is
        // complete, we should restart negotiation.
        StateWaitConfigurationPush,
        
        // We have requested that negotation be restarted with current
        // parameters, and are waiting for the read state to become fresh, and
        // when it does, we'll read the results and possibly declare the link
        // up.  We stay in this state as long as the link is up, constantly
        // rechecking the link.
        StateWaitAutonegFinish,

        // Autonegotiation resulted in some error. We generally come
        // to this state when an error has been detected during autoneg.
        // If the error can be resolved, we try and resolve it and 
        // restart autoneg. These errors are not reported to the higher
        // layer classes. This happens due to hardware bugs
        // like CSCdp90760
        StateWaitForAutonegErrorResolution,

        // We have been told to configure the link into loopback, which
        // short-circuits the remainder of autonegotiation since there is
        // no link partner with which to negotiate.
        StateLoopbackEnabled,

        // We didn't like our configuration, so we decided to disable the
        // link rather than negotiation.
        StateDisabled
        };
    State state_;

    // Phy-specific initialization state
    enum PhyInitState {
        PhyInitStateNull,
        
        // We have pushed out a request to reset the Phy.  This removes any
        // potential left over state from online diags.  We don't consider the
        // reset complete until the registers have been re-read.  The reset may
        // have invalidated our read cache.
        PhyInitStateWaitForPhyReset,
        
        // Then we cycle through a sequence of vendor specific registers for
        // the BCM5400.  This is needed for the B2 and C0 silicon.  It fixes a
        // tap power management bug that causes spurious bit errors, per Kelly
        // Coffey at Broadcom.
        PhyInitStateWaitBcm5400PowerFix,
        
        // Initializing the phy, we want to restart the master state machine
        // which will configure the phy to user desired operating state.
        PhyInitStateComplete
    };
    PhyInitState phyInitState_;

};

#if defined( NDEBUG ) || defined( pim_PimEthAutoNegotiator_cxx )

INLINE
PimEthAutoNegotiator::~PimEthAutoNegotiator() {
}

INLINE void
PimEthAutoNegotiator::debugLevel( int level ) {
    debugLevel_ = level;
}

INLINE int
PimEthAutoNegotiator::debugLevel() {
    return debugLevel_;
}

INLINE void
PimEthAutoNegotiator::pimPhyport( PimPhyport * pp ) {
    pimPhyport_ = pp;
}

INLINE PimPhyport *
PimEthAutoNegotiator::pimPhyport() {
    return pimPhyport_;
}

INLINE void
PimEthAutoNegotiator::ethAutoNegotiator( EthAutoNegotiator * ean ) {
    ethAutoNegotiator_ = ean;
}

INLINE EthAutoNegotiator *
PimEthAutoNegotiator::ethAutoNegotiator() {
    return ethAutoNegotiator_;
}

INLINE void
PimEthAutoNegotiator::ethDtePowerController( EthDtePowerController * 
                                             controller ) {
    ethDtePowerController_ = controller;
}

INLINE EthDtePowerController *
PimEthAutoNegotiator::ethDtePowerController() {
    return ethDtePowerController_;
}

INLINE u_int32 
PimEthAutoNegotiator::dtePowerOutOfControlCounts() {
    return dtePowerOutOfControlCounts_;
}

INLINE void
PimEthAutoNegotiator::dpmLinkupTimerStart() {
    dpmWaitLinkupTimer_ = clock_->microTime();
}

INLINE void
PimEthAutoNegotiator::dpmDelayCheckTimerStart() {
    dpmDelayCheckTimer_ = clock_->microTime();
}

INLINE bool
PimEthAutoNegotiator::dpmDelayCheckTimerTimeOut() {
    assert( dpmDelayCheckTimer_ != MicrosecondsNever );
    return ( ( clock_->microTime() - dpmDelayCheckTimer_ ) > 
              DelayCheckTimeWhenPowerDenied );
}

#endif /* Inline functions */

#endif /* PIM_PIMETHAUTONEGOTIATOR_H */

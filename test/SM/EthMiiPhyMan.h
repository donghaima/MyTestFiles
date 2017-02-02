/*
 * Copyright (c) 1998-2001, 2002 by Cisco Systems, Inc.
 * All rights reserved.
 */
#ifndef ETHMII_ETHMIIAUTONEGOTIATOR_H
#define ETHMII_ETHMIIAUTONEGOTIATOR_H

#include <gs/Name.h>
#include <event/Types.h>
#include <eth/Types.h>
#include <ethmii/Types.h>
#include <haleth/HalEthControlReg.h>
#include <haleth/HalEthAdReg.h>
#include <haleth/HalEth1000BasetControlReg.h>

// Implement Ethernet autonegotiation, given a device that supports "MII"
// (media independent interface).  MII is a standardized interface to the
// 10/100 Ethernet physical layer.  The interface is typically implemented in
// hardware.  For example, on "Hammerhead" (Catalyst 2948G), the Kirky (K1)
// ASIC connects to Astrodome 10/100 stubs, which connect to Broadcom BCM520B
// Ethernet PHY chips.  Software controls the Astrodomes through bulk
// read/write operations via ESMP.  Part of the Astrodome ESMP address space
// maps to the Broadcom PHY MII registers.  To drive this hardware, CxMan
// (Astrodome driver) implements an HalEthMiiDevice interface (per front-panel
// port), which when invoked causes ESMP writes to the Astrodome, which pushes
// the data into the Broadcom PHY.  Then CxMan instantiates an
// EthMiiPhyMan per front panel port, and attaches it to the Astrodome
// port's HalEthMiiDevice.


// Private warthog trampoline for my ethAutoNegotiator interface.
class EthMiiPhyMan {
    
  public:

    EthMiiPhyMan();
    ~EthMiiPhyMan();

    void name( char const * );
    char const * name();

    // Give me a device that I use to manipulate the Ethernet link at the
    // physical layer.
    void halEthMiiDevice( HalEthMiiDevice * );
    HalEthMiiDevice * halEthMiiDevice();

    // Tell me what type of mii phy I am supporting
    void ethMiiPhyType( EthMiiPhyType );
    EthMiiPhyType ethMiiPhyType();
    
    // Give me a clock.  I use it to check if it's time to re-read the phy
    // registers.
    void clock( Clock * );
    Clock * clock();
    
    // Generic interface to autonegotiation.  Use this to configure the
    // autonegotiation process, and find out the result.
    EthAutoNegotiator * ethAutoNegotiator();

    // Setting the PHY registers to enable or disable loopback mode.
    // Note: this function is asynchronise which means you need to wait for
    // write registers to complete ( via esmp packet ). In some cases, the 
    // the PHY need some time to get stable, so in general wait for ~200ms
    // before you actually test the loopback packet.
    void loopbackEnable( bool );

    u_int debugLevel();
    void debugLevel( u_int );

    u_int linkDownEvents();

    EthMiiPhyLoopbackState loopbackState();
    
    void showDebugInfo( Stream * s );

  private:
    
    // Private warthog trampoline for my ethAutoNegotiator interface.
    friend class EthMiiPhyManEthAutoNegotiator;
    EthMiiPhyManEthAutoNegotiator * ethAutoNegotiator_;

    // How do I manipulate the outside world.
    HalEthMiiDevice * halEthMiiDevice_;
    Clock * clock_;

    Name name_;

    // Local configuration.
    EthConfigurationMode ethConfigurationMode_;
    bool nextPageCapable_;
    EthFlowControlModeSet permittedFlowControlMode_;
    EthDuplexSet permittedDuplex_;
    EthLineSpeedSet permittedLineSpeed_;
    EthClockMode permittedClockMode_;
    EthFaultIndication localFaultIndication_;
    EthConnectorType connectorType_;
    u_int debugLevel_;
    EthMiiPhyType phyType_;
    
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


    struct StateStep {
        HalEthMiiRegNum regNum_;
        u_int16 val_;
    };

    static u_int const Bcm5400PowerFixMaxSteps = 7;
    static StateStep bcm5400PowerFixStepTable_[ Bcm5400PowerFixMaxSteps ];
    u_int bcm5400PowerFixStep_;
    
    RunMode runMode_;
    bool paramChanged_;         // Flag: has my configuration been mucked with
                                // since I last configured the HalEthMiiDevice?
    bool needToInitializePhy_;
    bool needToRestartAutoNegotiation_;
    bool needToWaitForLinkDownOnConfigChange_;

    Microseconds lastCompleteTime_; // When were all the things below valid?
    EthNegotiationStatus negotiationStatus_;
    EthNegotiationError negotiationError_;
    EthFaultIndication remoteFaultIndication_;
    EthLinkStatus linkStatus_;
    EthLineSpeed lineSpeed_;
    EthDuplex duplex_;
    EthFlowControlMode flowControlMode_;
    EthClockMode clockMode_;

    HalEthControlReg savedControlReg_;
    HalEthAdReg savedAdReg_;
    HalEth1000BasetControlReg saved1000BasetControlReg_;

    EthLoopback permittedLoopback_;
    EthLoopback loopback_;

    void runMode( RunMode );
    void changeParam();         // Whenever my configuration changes, this
                                // gets called.

    void restartStateMachine();
    void pushParamsIntoMiiAndRestartNegotiaton();
    void checkAutoNegComplete(); // Call to probe the MII and see if auto
                                 // negotiation is done.  If so, fill in the
                                 // above state with the results.  If not, fill
                                 // in the above state with NULLs.
    void autoMakeProgress(); // Helper for the above when autonegotiation is
                             // enabled.

    // Phy specific function
    void setMiiRegDefaults();     // Default phy configuration
    void disablePhyTransmission( bool ); // Phy specific tranmission disable
    u_int disconnectCounter();  // Phy specific implementation of
                                // how many link downs have occurred since
                                // phy power on
    void configurePhyConnector(); // Phy configuration that is needed for 
                                  // different connector types.
    void maybeMakePhyInitProgress();
    void restartPhyInitStateMachine();

    bool dpmDeviceDiscoveryCapable();
    void forcedDpmDeviceDiscoveryModeEnable( 
        EthDpmDeviceDiscoveryModeEnable );
    void forcedDpmDeviceDiscovery( EthDpmDeviceDiscovery, bool );

    EthDpmDeviceDiscoveryModeEnable dpmDeviceDiscoveryModeEnable();
    EthDpmDeviceDiscovery dpmDeviceDiscovery();
    EthDpmDeviceStatus dpmDeviceStatus();

    bool needToPushMoreDataToForceSpeedAndDuplex();

    void makeLoopbackProgress();

    bool autonegError();
    void maybeResolveAutonegErrors();

    void mrvl88E1040ResolveCrcErrorsAfterLinkUp();
    bool mrvl88E1040VendorReg31InRange();
    u_int mrvl88E1040CrcFixResetCounter_;

    bool needToPushMoreData_;
    EthMiiPhyLoopbackState loopbackState_;

    u_int16 defaultVendorRegValue( HalEthMiiRegNum ) const;
    u_int16 defaultBcm5208VendorRegValue( HalEthMiiRegNum ) const;
    u_int16 defaultBcm5218VendorRegValue( HalEthMiiRegNum ) const;
    u_int16 defaultLxt9782VendorRegValue( HalEthMiiRegNum ) const;
    u_int16 defaultMrvl88E1000VendorRegValue( HalEthMiiRegNum ) const;
    u_int16 defaultMrvl88E1040VendorRegValue( HalEthMiiRegNum ) const;


    u_int16 loopbackEnableControlRegRep( bool lb );

    // These steps are required as per the customer information sheet
    // for the Marvell 88E1000 Phys on Spacely.
    enum Mrvl88E1000LoopbackState {
        Mrvl88E1000LoopbackStateNull,
        Mrvl88E1000LoopbackEnableStarted,
        Mrvl88E1000LoopbackEnableStateStep1,
        Mrvl88E1000LoopbackEnableStateStep2,
        Mrvl88E1000LoopbackEnableStateStep3,
        Mrvl88E1000LoopbackEnableStateStep4,
        Mrvl88E1000LoopbackEnableStateStep5,
        Mrvl88E1000LoopbackEnableStateStep6,
        Mrvl88E1000LoopbackEnableStateComplete,
        Mrvl88E1000LoopbackDisableStarted, 
        Mrvl88E1000LoopbackDisableStateStep1,
        Mrvl88E1000LoopbackDisableStateStep2,
        Mrvl88E1000LoopbackDisableStateStep3,
        Mrvl88E1000LoopbackDisableStateComplete,
    };
    
    Mrvl88E1000LoopbackState mrvlLoopbackState_;

    enum Brcm5400AutoNegErrorFixState {
        Brcm5400AutoNegErrorFixStateNull,
        Brcm5400AutoNegErrorFixStateErrorDetected,
        Brcm5400AutoNegErrorFixStateStep1,
        Brcm5400AutoNegErrorFixStateStep2,
        Brcm5400AutoNegErrorFixStateStep3,
        Brcm5400AutoNegErrorFixStateErrorResolved
    };
    
    Brcm5400AutoNegErrorFixState brcm5400AutoNegErrorFixState_;
    u_int brcm5400AutoNegErrorFixCount_;

    void onNewConfiguration();

    void showBrcm5218DebugInfo( Stream * s );
    void showBrcm5208DebugInfo( Stream * s );
    void showBrcm5400DebugInfo( Stream * s );
    void showLxt9782DebugInfo( Stream * s );
    void showMrvl88E1000DebugInfo( Stream * s );
    void showMrvl88E1040DebugInfo( Stream * s );
};


#endif /* ETHMII_ETHMIIAUTONEGOTIATOR_H */

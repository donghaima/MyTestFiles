/*
 * Copyright (c) 1998-2001, 2002 by Cisco Systems, Inc.
 * All rights reserved.
 */

#include <gs/Time.h>
#include <gs/Stream.h>
#include <log/Log.h>
#include <store/Store.h>
#include <eth/EthAutoNegotiator.h>
#include <ethmii/EthMiiPhyMan.h>
#include <hal/HalVersionId.h>
#include <haleth/HalEthAuxControlReg.h>
#include <haleth/HalEthAnNextPageTransmitReg.h>
#include <haleth/HalEthBcm5208AuxMode2Reg.h>
#include <haleth/HalEthBcm5208AuxControlStatusReg.h>
#include <haleth/HalEthBcm5400ExtendedControlReg.h>
#include <haleth/HalEthBcm5218EnableDpmStatReg.h>
#include <haleth/HalEthBcm5218DpmReg.h>
#include <haleth/HalEthBcm5218AuxMode2Reg.h>
#include <haleth/HalEthBcm5218AuxControlReg.h>
#include <haleth/HalEthLxt9782TxControlOneReg.h>
#include <haleth/HalEthLxt9782PortConfigReg.h>
#include <haleth/HalEthStatusReg.h>
#include <haleth/HalEthMiiDevice.h>
#include <haleth/HalEth1000BasetStatusReg.h>
#include <haleth/HalEthExtendedStatusReg.h>
#include <haleth/HalEthMrvl88E1000PhySpecificCtrlReg.h>
#include <haleth/HalEthMrvl88E1040PhySpecificCtrlReg.h>
#include <haleth/HalEthBcm5400InterruptStatusReg.h>
#include <lemansman/LemansManPort.h>

enum {
    // The amount of time we consider MII register contents to be valid.  By
    // waiting this long between reads of MII regs, we avoid reading MII
    // registers repeatedly when the user asks for multiple related attributes.
    // This is important because on some systems, reading MII registers is very
    // slow.
    MiiPollInterval = 50 * 1000  
};


// The following table specifies a sequence of register writes which fixes a
// tap power management bug in the B2 and C0 silicon causing spurious bit
// errors.  The sequence is from Kelly Coffey at broadcom, kcoffey@broadcom.com
EthMiiPhyMan::StateStep
EthMiiPhyMan::bcm5400PowerFixStepTable_[ Bcm5400PowerFixMaxSteps ] = {
    { 0x18, 0x0c20 },
    { 0x17, 0x0012 },
    { 0x15, 0x1204 },
    { 0x17, 0x0013 },
    { 0x15, 0x1004 },
    { 0x17, 0x8006 },
    { 0x18, 0x0420 }
};

class EthMiiPhyManEthAutoNegotiator : public EthAutoNegotiator {

  public:
    
    EthMiiPhyManEthAutoNegotiator( EthMiiPhyMan * me ) {
        me_ = me; }
        
    virtual RunMode runMode();
    virtual void runMode( RunMode );

    virtual int debugLevel();
    virtual void debugLevel( int );

    virtual void restartAutoNegotiationOnConfigChange( bool );
    virtual void needToWaitForLinkDownOnConfigChange( bool );

    virtual EthConfigurationMode configurationMode();
    virtual void configurationMode( EthConfigurationMode );

    // Link configuration protocol input parameters.
    virtual void nextPageCapable( bool );
    virtual bool nextPageCapable();
    virtual void permittedFlowControlMode( EthFlowControlModeSet );
    virtual void permittedDuplex( EthDuplexSet );
    virtual void permittedLineSpeed( EthLineSpeedSet );
    virtual void localFaultIndication( EthFaultIndication );
    virtual void connectorType( EthConnectorType );
    virtual void permittedClockMode( EthClockMode );
    
    virtual EthFlowControlModeSet permittedFlowControlMode();
    virtual EthDuplexSet permittedDuplex();
    virtual EthLineSpeedSet permittedLineSpeed();
    virtual EthFaultIndication localFaultIndication();
    virtual EthConnectorType connectorType();
    virtual EthClockMode permittedClockMode();
    
    // Progress of link configuration protocol.
    virtual EthNegotiationStatus negotiationStatus();
    virtual EthNegotiationError negotiationError();
    virtual EthFaultIndication remoteFaultIndication();

    virtual bool dpmDeviceDiscoveryCapable();
    
    virtual void permittedDpmDeviceDiscoveryModeEnable( 
        EthDpmDeviceDiscoveryModeEnable );
    virtual EthDpmDeviceDiscoveryModeEnable 
        permittedDpmDeviceDiscoveryModeEnable();

    virtual EthDpmDeviceDiscoveryModeEnable dpmDeviceDiscoveryModeEnable();

    virtual void permittedDpmDeviceDiscovery( EthDpmDeviceDiscovery );
    virtual EthDpmDeviceDiscovery permittedDpmDeviceDiscovery();

    virtual EthDpmDeviceDiscovery dpmDeviceDiscovery();

    virtual void forcedDpmDeviceDiscoveryModeEnable( 
        EthDpmDeviceDiscoveryModeEnable );
    virtual void forcedDpmDeviceDiscovery(
        EthDpmDeviceDiscovery, bool );

    virtual void permittedLoopback( EthLoopback );
    virtual EthLoopback permittedLoopback();

    // Result of link configuration protocol.
    virtual EthLinkStatus linkStatus();
    virtual EthFlowControlMode flowControlMode();
    virtual EthDuplex duplex();
    virtual EthLineSpeed lineSpeed();
    virtual EthDpmDeviceStatus dpmDeviceStatus();
    virtual EthLoopback loopback();
    virtual EthClockMode clockMode();

    // Returns how many times the link has gone down since beginning.
    virtual u_int linkDownEvents();

    virtual void onNewConfiguration();

  private:
    
    EthMiiPhyMan * me_;
};

EthMiiPhyMan::EthMiiPhyMan() : paramChanged_( false ),
  needToInitializePhy_( false ), needToRestartAutoNegotiation_( true ),
  needToWaitForLinkDownOnConfigChange_( true ) {
    halEthMiiDevice_ = 0;
    clock_ = 0;
    runMode_ = RunModeReboot;
    debugLevel_ = 1;
    phyType_ = EthMiiPhyTypeNull;
    state_ = StateNull;
    phyInitState_ = PhyInitStateNull;
    lastCompleteTime_ = MicrosecondsNever;
    ethAutoNegotiator_ =
        new( ThisStore( this )) EthMiiPhyManEthAutoNegotiator( this );
    needToPushMoreData_ = false;
    loopbackState_ = EthMiiPhyLoopbackStateNull;
    mrvlLoopbackState_ = Mrvl88E1000LoopbackStateNull;
    brcm5400AutoNegErrorFixState_ = Brcm5400AutoNegErrorFixStateNull;
    brcm5400AutoNegErrorFixCount_ = 0;
    clockMode_ = EthClockModeNull;
}

EthMiiPhyMan::~EthMiiPhyMan() {
    assert( runMode_ == RunModeReboot );
    delete ethAutoNegotiator_;
}

char const * 
EthMiiPhyMan::name() {
    return name_.string();
}
 
void
EthMiiPhyMan::name( char const * nm ) {
    name_.string( nm );
}

void
EthMiiPhyMan::halEthMiiDevice( HalEthMiiDevice * emd ) {
    halEthMiiDevice_ = emd;
}

HalEthMiiDevice *
EthMiiPhyMan::halEthMiiDevice() {
    return halEthMiiDevice_;
}

void
EthMiiPhyMan::clock( Clock * c ) {
    clock_ = c;
}

Clock *
EthMiiPhyMan::clock() {
    return clock_;
}

EthAutoNegotiator *
EthMiiPhyMan::ethAutoNegotiator() {
    return ethAutoNegotiator_;
}

u_int
EthMiiPhyMan::debugLevel() {
    return debugLevel_;
}

void
EthMiiPhyMan::debugLevel( u_int d ) {
    debugLevel_ = d;
}

void
EthMiiPhyManEthAutoNegotiator::restartAutoNegotiationOnConfigChange( 
    bool b ) {
    me_->needToRestartAutoNegotiation_ = b;
}

void
EthMiiPhyManEthAutoNegotiator::needToWaitForLinkDownOnConfigChange( 
    bool b ) {
    me_->needToWaitForLinkDownOnConfigChange_ = b;
}

RunMode
EthMiiPhyManEthAutoNegotiator::runMode() {
    return me_->runMode_;
}

void
EthMiiPhyManEthAutoNegotiator::runMode( RunMode rm ) {
    me_->runMode( rm );
}

// Currently, I don't do anything with the debug level
// passed to ethautoneg... 
int
EthMiiPhyManEthAutoNegotiator::debugLevel() {
    return 0;
}

void
EthMiiPhyManEthAutoNegotiator::debugLevel( int level ) {
    // Nothing.
}

EthConfigurationMode
EthMiiPhyManEthAutoNegotiator::configurationMode() {
    return me_->ethConfigurationMode_;
}

void
EthMiiPhyManEthAutoNegotiator::configurationMode(
    EthConfigurationMode ecm ) {
    if( ecm != me_->ethConfigurationMode_ ) {
        me_->changeParam();
        me_->ethConfigurationMode_ = ecm;
    }
}

void
EthMiiPhyManEthAutoNegotiator::connectorType( EthConnectorType type ) {
    assert( type != EthConnectorTypeNull );
    if( me_->connectorType_ != type ) {
        me_->changeParam();
        me_->connectorType_ = type;
    }
}

EthConnectorType
EthMiiPhyManEthAutoNegotiator::connectorType() {
    return me_->connectorType_;
}


void
EthMiiPhyManEthAutoNegotiator::nextPageCapable( bool npc ) {
    if( me_->nextPageCapable_ != npc ) {
        me_->changeParam();
        me_->nextPageCapable_ = npc;
    }
}

bool
EthMiiPhyManEthAutoNegotiator::nextPageCapable() {
    return me_->nextPageCapable_;
}

void
EthMiiPhyManEthAutoNegotiator::permittedFlowControlMode(
    EthFlowControlModeSet efcms ) {
    if( me_->permittedFlowControlMode_ != efcms ) {
        me_->changeParam();
        me_->permittedFlowControlMode_ = efcms;
    }
}

void
EthMiiPhyManEthAutoNegotiator::permittedDuplex( EthDuplexSet eds ) {
    if( me_->permittedDuplex_ != eds ) {
        me_->changeParam();
        me_->permittedDuplex_ = eds;
    }
}

void
EthMiiPhyManEthAutoNegotiator::permittedLineSpeed(
    EthLineSpeedSet elss ) {
    if( me_->permittedLineSpeed_ != elss ) {
        me_->changeParam();
        me_->permittedLineSpeed_ = elss;
    }
}

void
EthMiiPhyManEthAutoNegotiator::permittedClockMode( EthClockMode ecm ) {
    if( me_->permittedClockMode_ != ecm ) {
        me_->changeParam();
        me_->permittedClockMode_ = ecm;
    }
}

void
EthMiiPhyManEthAutoNegotiator::localFaultIndication(
    EthFaultIndication efi ) {
    if( me_->localFaultIndication_ != efi ) {
        me_->changeParam();
        me_->localFaultIndication_ = efi;
    }
    
}

bool 
EthMiiPhyManEthAutoNegotiator::dpmDeviceDiscoveryCapable() {

    return me_->dpmDeviceDiscoveryCapable();
}

void
EthMiiPhyManEthAutoNegotiator::permittedDpmDeviceDiscoveryModeEnable(
    EthDpmDeviceDiscoveryModeEnable enable ) {
    // This interface is depreciated in K2, like other derived class 
    // of EthAutoNegotiator, we assert no one use it.
    assert( 0 );
}

EthDpmDeviceDiscoveryModeEnable
EthMiiPhyManEthAutoNegotiator::permittedDpmDeviceDiscoveryModeEnable() {
    // This interface is depreciated in K2, like other derived class 
    // of EthAutoNegotiator, we assert no one use it.
    assert( 0 );
    return EthDpmDeviceDiscoveryModeEnableNull;
}

void
EthMiiPhyManEthAutoNegotiator::permittedDpmDeviceDiscovery( 
    EthDpmDeviceDiscovery discovery ) {
    // This interface is depreciated in K2, like other derived class 
    // of EthAutoNegotiator, we assert no one use it.
    assert( 0 );
}

EthDpmDeviceDiscovery
EthMiiPhyManEthAutoNegotiator::permittedDpmDeviceDiscovery() {
    // This interface is depreciated in K2, like other derived class 
    // of EthAutoNegotiator, we assert no one use it.
    assert( 0 );
    return EthDpmDeviceDiscoveryNull;
}

EthFlowControlModeSet
EthMiiPhyManEthAutoNegotiator::permittedFlowControlMode() {
    return me_->permittedFlowControlMode_;
}

EthDuplexSet
EthMiiPhyManEthAutoNegotiator::permittedDuplex() {
    return me_->permittedDuplex_;
}

EthLineSpeedSet
EthMiiPhyManEthAutoNegotiator::permittedLineSpeed() {
    return me_->permittedLineSpeed_;
}

EthClockMode
EthMiiPhyManEthAutoNegotiator::permittedClockMode() {
    return me_->permittedClockMode_;
}

EthFaultIndication
EthMiiPhyManEthAutoNegotiator::localFaultIndication() {
    return me_->localFaultIndication_;
}

EthNegotiationStatus
EthMiiPhyManEthAutoNegotiator::negotiationStatus() {
    if( me_->runMode_ < RunModeOperating ) {
        return EthNegotiationStatusNotStarted;
    }
    me_->checkAutoNegComplete();
    return me_->negotiationStatus_;
}

EthNegotiationError
EthMiiPhyManEthAutoNegotiator::negotiationError() {
    return me_->negotiationError_;
}

EthFaultIndication
EthMiiPhyManEthAutoNegotiator::remoteFaultIndication() {
    me_->checkAutoNegComplete();
    return me_->remoteFaultIndication_;
}

EthLinkStatus
EthMiiPhyManEthAutoNegotiator::linkStatus() {
    me_->checkAutoNegComplete();
    return me_->linkStatus_;
}

EthFlowControlMode
EthMiiPhyManEthAutoNegotiator::flowControlMode() {
    me_->checkAutoNegComplete();
    return me_->flowControlMode_;
}

EthDuplex
EthMiiPhyManEthAutoNegotiator::duplex() {
    me_->checkAutoNegComplete();
    return me_->duplex_;
}

EthLineSpeed
EthMiiPhyManEthAutoNegotiator::lineSpeed() {
    me_->checkAutoNegComplete();
    return me_->lineSpeed_;
}

EthClockMode
EthMiiPhyManEthAutoNegotiator::clockMode() {
    me_->checkAutoNegComplete();
    return me_->clockMode_;
}

EthLoopback
EthMiiPhyManEthAutoNegotiator::loopback() {
    me_->checkAutoNegComplete();
    return me_->loopback_;
}

EthDpmDeviceDiscoveryModeEnable
EthMiiPhyManEthAutoNegotiator::dpmDeviceDiscoveryModeEnable() {
    assert( dpmDeviceDiscoveryCapable());
    return me_->dpmDeviceDiscoveryModeEnable();
}

EthDpmDeviceDiscovery
EthMiiPhyManEthAutoNegotiator::dpmDeviceDiscovery() {
    assert( dpmDeviceDiscoveryCapable());
    return me_->dpmDeviceDiscovery();
}

void
EthMiiPhyManEthAutoNegotiator::forcedDpmDeviceDiscoveryModeEnable( 
    EthDpmDeviceDiscoveryModeEnable mode ) {
    assert( dpmDeviceDiscoveryCapable());
    me_->forcedDpmDeviceDiscoveryModeEnable( mode );
}

void
EthMiiPhyManEthAutoNegotiator::forcedDpmDeviceDiscovery(
    EthDpmDeviceDiscovery discovery, bool restartAutoNeg ) {
    assert( dpmDeviceDiscoveryCapable());
    me_->forcedDpmDeviceDiscovery( discovery, restartAutoNeg );
}

EthDpmDeviceStatus
EthMiiPhyManEthAutoNegotiator::dpmDeviceStatus() {
    me_->checkAutoNegComplete();
    return me_->dpmDeviceStatus();
}

void
EthMiiPhyManEthAutoNegotiator::permittedLoopback( EthLoopback l ) {
    if( me_->permittedLoopback_ != l ) {
        me_->changeParam();

        assert( l == EthLoopbackOn || l == EthLoopbackOff );

        me_->permittedLoopback_ = l;
    }
}

EthLoopback
EthMiiPhyManEthAutoNegotiator::permittedLoopback() {
    return me_->permittedLoopback_;
}

u_int
EthMiiPhyManEthAutoNegotiator::linkDownEvents() {
    me_->checkAutoNegComplete();
    return me_->linkDownEvents();
}

void
EthMiiPhyManEthAutoNegotiator::onNewConfiguration() {
    me_->onNewConfiguration();
}

u_int
EthMiiPhyMan::linkDownEvents() {
    return halEthMiiDevice_->linkDownEvents();
}

void
EthMiiPhyMan::runMode( RunMode rm ) {
    if( debugLevel_ >= 3 ) {
        LogID(( "EthMiiPhyMan: runMode = %s",
                RunModeToString( rm ) ));
    }
    while( runMode_ != rm ) {
        if( runMode_ < rm ) {
            // booting.
            switch( runMode_ ) {

              default:
                assert( 0 );
                
              case RunModeReboot:

                // EthMiiPhy supports several types of phys.  While the first
                // 16 registers have the same definitions and uses, the last 16
                // are used for vendor specific operation, e.g. support FX,
                // disabling transmission, and counting link up/downs which is
                // not defined in the 802.3 standard.  Therefore, we need to
                // know what phy we're managing in order to correctly support
                // the above actions.
                assert( phyType_ != EthMiiPhyTypeNull );
                
                changeParam();
                ethConfigurationMode_ = EthConfigurationModeDisabled;
                nextPageCapable_ = false;
                permittedFlowControlMode_.empty( true );
                permittedDuplex_.empty( true );
                permittedLineSpeed_.empty( true );
                localFaultIndication_ = EthFaultIndicationNone;
                connectorType_ = EthConnectorTypeNull;
                permittedClockMode_ = EthClockModeNull;

                runMode_ = RunModeReset;
                break;
                
              case RunModeReset:
                // When coming out of reset, it's possible that someone (such
                // as CxMan diags) has mucked with the MII registers while
                // we were reset.  So, remember reprogram all the MII registers
                // when we thaw.  (This may be the cause of CSCdk77479.)
                changeParam();
                needToInitializePhy_ = true;
                runMode_ = RunModeFrozen;
                break;

              case RunModeFrozen:
                runMode_ = RunModeOperating;
                assert( halEthMiiDevice_ );
                if( paramChanged_ ) {
                    restartStateMachine();
                }
                break;
            }
        } else {
            // shutting down.
            switch( runMode_ ) {
              default:
                assert( 0 );
                return;
              case RunModeOperating:
                runMode_ = RunModeFrozen;
                break;
              case RunModeFrozen:
                runMode_ = RunModeReset;
                break;
              case RunModeReset:
                runMode_ = RunModeReboot;
                break;
            }
        }
    }
}

void
EthMiiPhyMan::changeParam() {
    assert( runMode_ < RunModeOperating );
    if( linkStatus_ == EthLinkStatusUp ) {
        linkStatus_ = EthLinkStatusDown;
    }
    paramChanged_ = true;
    lastCompleteTime_ = MicrosecondsNever;
}

void
EthMiiPhyMan::restartStateMachine() {
    
    paramChanged_ = false;

    // Next time the user asks us how we're doing, we have to re-read
    // everything.  lastCompleteTime_==MicrosecondsNever causes this to
    // happen.  It should already be set this way, because we should only
    // get here when a negotiation parameter changes.
    assert( lastCompleteTime_ == MicrosecondsNever );

    // First configure all mii register defaults in case diagnostics
    // mucked with the values.
    if( needToInitializePhy_ ) {
        SM_throwEvent( initPhy );
        restartPhyInitStateMachine();
        // state_ = StateWaitPhyInit;
    } else {
        if( needToWaitForLinkDownOnConfigChange_ ) {
            SM_throwEvent( WaitLinkDown );
            // state_ = StateWaitLinkDown;
            // Then, ask the phy to take the link down,
            disablePhyTransmission( true );
        } else {
            pushParamsIntoMiiAndRestartNegotiaton();
        }
            
        // From here, the state machine (see autoMakeProgress()) will wait for
        // the link to really go down, and then will reconfigure
        // autonegotiation.
    }    
}

void
EthMiiPhyMan::pushParamsIntoMiiAndRestartNegotiaton() {
    
    HalEthControlReg control;
    HalEthAdReg ad;
    HalEth1000BasetControlReg base1000tControl;
    control.rep( 0 );
    ad.rep( 0 );
    base1000tControl.rep( 0 );

    assert( permittedLoopback_ != EthLoopbackOn );

    switch( ethConfigurationMode_ ) {
      default:
        assert( 0 );

      case EthConfigurationModeDisabled:
      disable:
        // Phy is currently disabled at this point. We do not want to continue
        // configuring the phy for forced or autonegotiation operating modes.
        assert( state_ == StateWaitLinkDown );
        SM_throwEvent( Disabled );
        // state_ = StateDisabled;
        return;
       
      case EthConfigurationModeForced:
        needToPushMoreData_ = false;

        if( permittedDuplex_.member( EthDuplexFull )) {
            control.duplex( EthDuplexFull );
        } else if( permittedDuplex_.member( EthDuplexHalf ) ) {
            control.duplex( EthDuplexHalf );
        } else {
            goto disable;
        }
        if( permittedLineSpeed_.member( EthLineSpeed100Mbps )) {
            control.lineSpeed( EthLineSpeed100Mbps );
        } else if( permittedLineSpeed_.member( EthLineSpeed10Mbps )) {
            control.lineSpeed( EthLineSpeed10Mbps );
        } else if( permittedLineSpeed_.member( EthLineSpeed1Gbps )) {
            control.lineSpeed( EthLineSpeed1Gbps );
        } else {
            goto disable;
        }
        if( permittedFlowControlMode_.member( EthFlowControlModeNone )) {
            // Ok, this is all the hardware can do.
        } else {
            goto disable;
        }

        switch( phyType_ ) {
            
          case EthMiiPhyTypeBcm5218: 
          case EthMiiPhyTypeBcm5208:
          case EthMiiPhyTypeBcm5400:
          case EthMiiPhyTypeLxt9782: { 
            // nothing specific needs to be done for these phys
            break;
          }

          case EthMiiPhyTypeMrvl88E1000: {
            // The forced speed and duplex configurations on this Phy have 
            // some hardware bugs and we need to go through some extra steps
            // to work around them.
            // We first need to disable Auto-Mdix and reset the phy.
            // Then disable auto neg, set speed and duplex and reset 
            // the phy. The reset bit in the control register should be set
            // in the same write operation when writing the new data.

            HalEthMrvl88E1000PhySpecificCtrlReg auxCtrlReg;
            auxCtrlReg.rep( 0 );
            if( permittedLineSpeed_.member( EthLineSpeed1Gbps )) {
                auxCtrlReg.mdiCrossoverMode( HalEthMrvl88E1000ModeAutoMdix );
            } else {
                auxCtrlReg.mdiCrossoverMode( HalEthMrvl88E1000ModeManualMdix );
            }
            halEthMiiDevice_->writableVendorReg( 
                HalEthMrvl88E1000PhySpecificCtrlRegNum, auxCtrlReg.rep() );

            control.reset( true );
            SM_throwEvent( WaitConfigurationPush );
            // state_ = StateWaitConfigurationPush;
            needToPushMoreData_ = true;
            break;
          }

          case EthMiiPhyTypeMrvl88E1040: {
            // We need to disable Auto-Mdix when speed is forced to 10/100
            // Mb/s; If the Auto-Mdix is enabled in forced 10/100 Mb/s 
            // operation, link takes long time to come up if the link partner 
            // is auto negotiating. This is not recommended by IEEE
            HalEthMrvl88E1040PhySpecificCtrlReg ctrlReg;
            ctrlReg.rep( 0 );
            if( permittedLineSpeed_.member( EthLineSpeed1Gbps )) {
                ctrlReg.mdiCrossoverMode( HalEthMrvl88E1040ModeAutoMdix );
            } else {
                ctrlReg.mdiCrossoverMode( HalEthMrvl88E1040ModeManualMdix );
            }
            halEthMiiDevice_->writableVendorReg( 
                HalEthMrvl88E1040PhySpecificCtrlRegNum, ctrlReg.rep() );

            control.reset( true );
            SM_throwEvent( WaitConfigurationPush );
            // state_ = StateWaitConfigurationPush;
            needToPushMoreData_ = true;
            break;
          }

          default:
            assert( 0 );
        }

        state_ = StateWaitConfigurationPush; /// ?????????????????

        break;
        
      case EthConfigurationModeAuto: {

        HalEthStatusReg stat = halEthMiiDevice_->statusReg();
        
        bool halfOk = permittedDuplex_.member( EthDuplexHalf );
        bool fullOk = permittedDuplex_.member( EthDuplexFull );
        bool tenOk = permittedLineSpeed_.member( EthLineSpeed10Mbps );
        bool hunOk = permittedLineSpeed_.member( EthLineSpeed100Mbps );
        bool thouOk = permittedLineSpeed_.member( EthLineSpeed1Gbps );

        // Make sure I haven't been asked to negotiate something ridiculous.
        // We could be even more rigorous and make sure we aren't trying to
        // negotiate a speed and duplex not supported by the particular phy.

        if( localFaultIndication_ == EthFaultIndicationNull ||
             (!halfOk && !fullOk) || (!tenOk && !hunOk && !thouOk) ||
            !(permittedFlowControlMode_.member( EthFlowControlModeNone )) ) {
            
            // I can't negotiate this.  Keep the link disabled until I've
            // been configured sensibly.
            goto disable;
        }

        // Set up 10/100 capabilities for autonegotation.  
        control.autoNegotiationEnable( true );
        ad.remoteFault( localFaultIndication_ != EthFaultIndicationNone );
        ad.base10tHalfDuplexCapable( stat.base10tHalfDuplexCapable() &&
                                        tenOk && halfOk );
        ad.base10tFullDuplexCapable( stat.base10tFullDuplexCapable() &&
                                        tenOk && fullOk );
        ad.base100txHalfDuplexCapable( stat.base100txHalfDuplexCapable() &&
                                          hunOk && halfOk );
        ad.base100txFullDuplexCapable( stat.base100txFullDuplexCapable() &&
                                          hunOk && fullOk );
        // This assert is here because I don't understand 100BASE-T4, so I'm
        // not sure if the logic for handling it is right.  -kjd Oct 22 1998
        // If you're hitting this assert, it could be possible that your 
        // phys did not come out of hardware reset. -Anshul Nov 22 2000.
        assert( !stat.base100t4Capable() );
        ad.base100t4Capable( stat.base100t4Capable() && hunOk && halfOk );
        ad.selectorField( HalEthAdSelectorField802_3 );


        // See eth/EthSoftwareAutoNegotiator.cxx for an explanation of
        // how permittedFlowControlModes are translated into the
        // two pause control values. Today we only expect to support
        // flowcontrol on 1000BaseT ports.
        if( permittedFlowControlMode_.member( EthFlowControlModeRecvOnly )) {
            
            assert( permittedLineSpeed_.member( EthLineSpeed1Gbps ) );
            ad.asymmetricPauseToPartner( true );
            ad.symmetricPause( true );
        } else if( permittedFlowControlMode_.member(
            EthFlowControlModeSendRecv )) {
            
            assert( permittedLineSpeed_.member( EthLineSpeed1Gbps ) );
            ad.asymmetricPauseToPartner( false );
            ad.symmetricPause( true );
        } else if( permittedFlowControlMode_.member(
            EthFlowControlModeSendOnly )) {
            
            assert( permittedLineSpeed_.member( EthLineSpeed1Gbps ) );
            ad.asymmetricPauseToPartner( true );
            ad.symmetricPause( false );
        } else if( permittedFlowControlMode_.member(
            EthFlowControlModeNone )) {
            
            ad.asymmetricPauseToPartner( false );
            ad.symmetricPause( false );
        } else {
            goto disable;
        }

        // Add the local fault indication last.

        // Set up 1000BaseT capabilities for autonegotiation only if we're
        // permitting it.  We do this because we're not sure how older phys
        // which don't have the 1000BaseT registers will respond to writing
        // unsupported register addresses although according to the IEEE it
        // should do no harm.
        if( thouOk ) {
            HalEthExtendedStatusReg extendedStat =
                halEthMiiDevice_->extendedStatusReg();
            HalEth1000BasetStatusReg base1000tStat =
                halEthMiiDevice_->halEth1000BasetStatusReg();
           
            // In software, we treat speed and duplex as different
            // attributes but the Phy hardware uses them in pairs.
            // As of now, we do not support 1000/Half and it is unlikely
            // that this will be supported in the near future.
            // For 10/100/1000 Phys, we do support 10/half and 100/half,
            // but that does not apply to speed 1000. For now, I am manually
            // forcing the phys not to advertise 1000/half. One way of
            // fixing this would be to have Nmp or Ios send us permitted 
            // speed/duplex pairs rather than send them as a set of unrelated
            // attributes. - Anshul, Jan 27, 2002.
            base1000tControl.advertise1000BasetHalfCapability( false );

            base1000tControl.advertise1000BasetFullCapability(
                thouOk & fullOk );
        }
    
        switch( phyType_ ) {
            
          case EthMiiPhyTypeBcm5208:
          case EthMiiPhyTypeBcm5218:
          case EthMiiPhyTypeBcm5400: 
          case EthMiiPhyTypeLxt9782:
            break;
            
          case EthMiiPhyTypeMrvl88E1000:
            HalEthMrvl88E1000PhySpecificCtrlReg auxCtrlReg;
            auxCtrlReg.rep( 0 );
            auxCtrlReg.mdiCrossoverMode( HalEthMrvl88E1000ModeAutoMdix );
            halEthMiiDevice_->writableVendorReg( 
                HalEthMrvl88E1000PhySpecificCtrlRegNum, auxCtrlReg.rep() );
            
            control.reset( true );
            break;

          case EthMiiPhyTypeMrvl88E1040:
            HalEthMrvl88E1040PhySpecificCtrlReg ctrlReg;
            ctrlReg.rep( 0 );
            ctrlReg.mdiCrossoverMode( HalEthMrvl88E1040ModeAutoMdix );
            halEthMiiDevice_->writableVendorReg( 
                HalEthMrvl88E1040PhySpecificCtrlRegNum, ctrlReg.rep() );
            
            break;

          default:
            assert( 0 );
        }
        SM_throwEvent( WaitConfigurationPush );
        // state_ = StateWaitConfigurationPush;
        break;
      }
    }

    if( permittedLineSpeed_.member( EthLineSpeed1Gbps ) ) {
        switch( permittedClockMode_ ) {

          case EthClockModeForcedMaster:
            base1000tControl.manualMasterSlaveConfigEnable( true );
            base1000tControl.masterSlaveConfigValue( 
                HalEth1000BasetConfigValueMaster );
            break;

          case EthClockModeForcedSlave:
            base1000tControl.manualMasterSlaveConfigEnable( true );
            base1000tControl.masterSlaveConfigValue( 
                HalEth1000BasetConfigValueSlave );
            break;

          case EthClockModePrefMaster:
            base1000tControl.manualMasterSlaveConfigEnable( false );
            base1000tControl.portMode( HalEth1000BasetPortModeRepeater );
            break;

          case EthClockModePrefSlave:
            base1000tControl.manualMasterSlaveConfigEnable( false );
            base1000tControl.portMode( HalEth1000BasetPortModeDte );
            break;

          case EthClockModeAuto:
            // Setting clock mode to auto defaults to pref-master for all
            // our phys. If in future, some other phy needs a different
            // default mode, then it could be set based on the phy type.
            base1000tControl.manualMasterSlaveConfigEnable( false );
            base1000tControl.portMode( HalEth1000BasetPortModeRepeater );
            break;

          case EthClockModeNull:
            assert( 0 ); 
            break;

          default:
            assert( 0 );
            break;

        }
    }

    assert( state_ != StateDisabled );

    // Configure any phy specific regs.
    // This code path always is to bring up the link.
    disablePhyTransmission( false );  

    configurePhyConnector();
    
    // Must write other registers before restarting autonegotiation, so that
    // autonegotiation starts out with the right advertisement.
    halEthMiiDevice_->adReg( ad );
    halEthMiiDevice_->controlReg( control );
    halEthMiiDevice_->halEth1000BasetControlReg( base1000tControl );
        
    // Stash these away; we'll refer to them later when autonegotiation
    // completes.
    savedControlReg_ = control;
    savedAdReg_ = ad;
    saved1000BasetControlReg_ = base1000tControl;

}

void
EthMiiPhyMan::checkAutoNegComplete() {

    Microseconds now = clock_->microTime();
    if( lastCompleteTime_ != MicrosecondsNever &&
        lastCompleteTime_ + MiiPollInterval > now ) {
        // Yes, we checked recently, and we're done.
        return;
    }

    // Fill in some assumptions first.  We'll update them below as we learn
    // more.
    negotiationStatus_ = EthNegotiationStatusNotStarted;
    negotiationError_ = EthNegotiationErrorNull;
    remoteFaultIndication_ = EthFaultIndicationNull;
    linkStatus_ = EthLinkStatusDown;
    lineSpeed_ = EthLineSpeedNull;
    duplex_ = EthDuplexNull;
    loopback_ = EthLoopbackNull;
    flowControlMode_ = EthFlowControlModeNull;

    // If we're not operating, then our assumptions above were all right!
    if( runMode_ < RunModeOperating ) {
        return;
    }

    lastCompleteTime_ = now;

    State oldState;
    do {
        oldState = SM->state();
        SM->run();
    } while ( oldState != SM->state() );
}


State StateWaitPhyInit() functionBeforeRun() {
      maybeMakePhyInitProgress();
}

State StateWaitLinkDown() functionBeforeRun() {
    // We are waiting for the MII to be told to take the link down.
    // See if it has happened.
    if( halEthMiiDevice_->hardwareWriteVersion() !=
        halEthMiiDevice_->softwareWriteVersion() ) {
        // Not yet.
        return;  
    }
    
    // Yes, it has.  Now configure autonegotiation, and fall through.
    // We don't need to configure autonegotiation if the port is
    // going into loopback, though.
    if( permittedLoopback_ == EthLoopbackOn ) {
        SM_throwEvent( LoopBackEnabled );
        state_ = StateLoopbackEnabled;
    } else {
        pushParamsIntoMiiAndRestartNegotiaton();
    }
}

State StateDisabled() functionBeforeRun() {

    switch( ethConfigurationMode_ ) {
      default:
        assert( 0 );
        
      case EthConfigurationModeDisabled:
        // We know the port is down.  Nothing to check or fill in,
        // except indicate that we aren't planning to do more
        // negotiation.
        negotiationStatus_ = EthNegotiationStatusSuccessful;
        break;
        
      case EthConfigurationModeForced:
      case EthConfigurationModeAuto:
        // We're misconfigured which is how we ended up here, so we
        // indicate that via status InProgress.  If the user of
        // EthMiiPhyMan wanted to disable the phy, it should have set
        // our EthConfigurationMode to disable
        negotiationStatus_ = EthNegotiationStatusInProgress;
        break;
    }
}

State StateLoopbackEnabled() functionBeforeRun() {
    // The port is going to be configured into loopback.  Since
    // this means we don't have a link partner, autonegotiation
    // is effectively concluded.
    HalEthControlReg control;
    control.rep( loopbackEnableControlRegRep( true ) );
    
    negotiationStatus_ = EthNegotiationStatusSuccessful;
    linkStatus_ = EthLinkStatusUp;
    lineSpeed_ = control.lineSpeed();
    loopback_ = EthLoopbackOn;
    duplex_ = control.duplex();
    flowControlMode_ = EthFlowControlModeNone;
    remoteFaultIndication_ = EthFaultIndicationNone;
}
    
State StateWaitConfigurationPush() functionBeforeRun() {
    if( needToPushMoreDataToForceSpeedAndDuplex() ) {
        return;
    } else {
        // else goto the next case and check if autoneg finished.
        waitAutonegFinsih();
    }
}

// transition for statewaitconfigurationPush to WaitAutonegFinish
// triggered by autoMakeProgress.       
Transition in StateWaitConfigurationPush() {
    // We are waiting for our negotiation parameters to get pushed out
    // to the MII device.  See if it has happened.
    if( halEthMiiDevice_->hardwareWriteVersion() !=
        halEthMiiDevice_->softwareWriteVersion() ) {
        break;
    }
        
    savedControlReg_.restartAutoNegotiation( 
        needToRestartAutoNegotiation_ );
    halEthMiiDevice_->controlReg( savedControlReg_ );
    SM_throwEvent( StateWaitAutonegFinish );
}

// Transition in WaitConfigurationPush and 
// triggered by waitAutonegFinish() 
Transition waitAutonegFinish() {
    switch( ethConfigurationMode_ ) {
      case EthConfigurationModeDisabled:
        // The disabled mode is handled earlier in the state machine
        // before we try to configure autonegotiation or a forced
        // configuration.
      default:
        assert( 0 );
        
      case EthConfigurationModeForced: {
          // In case of forced configuration, you cannot ask for dpm
          // device discovery.  This is not supported as of BCM5218.
          // We are going to assert out if such a case arises.  You
          // can possibly enable the DPM mode, but can't turn the
          // discovery on.
          HalEthStatusReg status = halEthMiiDevice_->statusReg();
          // If the link is in loopback mode, that's because we're not
          // even trying to bring it up.
          if( status.linkStatus() == EthLinkStatusUp ) {
              // and link is up.
              negotiationStatus_ = EthNegotiationStatusSuccessful;
              assert( !savedControlReg_.autoNegotiationEnable() );
              linkStatus_ = EthLinkStatusUp;
              lineSpeed_ = savedControlReg_.lineSpeed();
              loopback_ = permittedLoopback_;
              duplex_ = savedControlReg_.duplex();
              flowControlMode_ = EthFlowControlModeNone;
              remoteFaultIndication_ = EthFaultIndicationNone;
              HalEth1000BasetStatusReg status1000Baset =
                  halEthMiiDevice_->halEth1000BasetStatusReg();
              if( lineSpeed_ == EthLineSpeed1Gbps ) {
                  if( status1000Baset.localTransmitterIsMaster() ) {
                      clockMode_ = EthClockModeMaster;
                  } else {
                      clockMode_ = EthClockModeSlave;
                  }
              } else {
                  clockMode_ = EthClockModeNone;
              }
          } else {
              // we're still trying to force the link
              negotiationStatus_ = EthNegotiationStatusInProgress;
          }
          
          break;
      }
      
      case EthConfigurationModeAuto:
        autoMakeProgress();
        break;
    }
}
    
State StateWaitAutonegFinish() functionBeforeRun() {
    waitAutonegFinish();
}

// transition for WaitAutonegFinish
// triggered by Event autoMakeProgress
Transition in StateWaitAutonegFinish() {
    
    assert( savedControlReg_.autoNegotiationEnable() );
    
    if( halEthMiiDevice_->hardwareReadVersion() !=
        halEthMiiDevice_->softwareWriteVersion() ) {
        break;
    }
    
    HalEthStatusReg status = halEthMiiDevice_->statusReg();
    linkStatus_ = status.linkStatus();
    if( linkStatus_ == EthLinkStatusUp ) {
        if( status.autoNegotiationComplete() ) {
            if( debugLevel_ >= 3 ) {
                LogID(( "EthMiiPhyMan: MII indicates "
                        "autonegotiation is complete" ));
            }
            HalEthAdReg partner = halEthMiiDevice_->linkPartnerAdReg();
            HalEth1000BasetStatusReg status1000Baset =
                halEthMiiDevice_->halEth1000BasetStatusReg();
            // I'm not sure how to handle 100BASE-T4.  -kjd Oct 22 1998
            assert( !savedAdReg_.base100t4Capable() );
            negotiationStatus_ = EthNegotiationStatusSuccessful;
            if( saved1000BasetControlReg_.advertise1000BasetFullCapability() &&
                status1000Baset.linkPartner1000BasetFullDuplexCapable() ) {
                lineSpeed_ = EthLineSpeed1Gbps;
                duplex_ = EthDuplexFull;
            } else if(
                saved1000BasetControlReg_.advertise1000BasetHalfCapability() &&
                status1000Baset.linkPartner1000BasetHalfDuplexCapable() ) {
                lineSpeed_ = EthLineSpeed1Gbps;
                duplex_ = EthDuplexHalf;
            } else if( savedAdReg_.base100txFullDuplexCapable() && 
                       partner.base100txFullDuplexCapable() ) {
                lineSpeed_ = EthLineSpeed100Mbps;
                duplex_ = EthDuplexFull;
            } else if( savedAdReg_.base100txHalfDuplexCapable() &&
                       partner.base100txHalfDuplexCapable() ) {
                lineSpeed_ = EthLineSpeed100Mbps;
                duplex_ = EthDuplexHalf;
            } else if( savedAdReg_.base10tFullDuplexCapable() &&
                       partner.base10tFullDuplexCapable() ) {
                lineSpeed_ = EthLineSpeed10Mbps;
                duplex_ = EthDuplexFull;
            } else if( savedAdReg_.base10tHalfDuplexCapable() &&
                       partner.base10tHalfDuplexCapable() ) {
                lineSpeed_ = EthLineSpeed10Mbps;
                      duplex_ = EthDuplexHalf;
            } else {
                // No common modes.
                if( ((savedAdReg_.base10tFullDuplexCapable() ||
                      savedAdReg_.base10tHalfDuplexCapable()) &&
                     (partner.base10tFullDuplexCapable() ||
                      partner.base10tHalfDuplexCapable())) ||
                    ((savedAdReg_.base100txFullDuplexCapable() ||
                      savedAdReg_.base100txHalfDuplexCapable()) &&
                     (partner.base100txFullDuplexCapable() ||
                      partner.base100txHalfDuplexCapable())) ||
                    ((saved1000BasetControlReg_.advertise1000BasetHalfCapability() ||
                      saved1000BasetControlReg_.advertise1000BasetFullCapability() ) &&
                     (status1000Baset.linkPartner1000BasetHalfDuplexCapable() ||
                      status1000Baset.linkPartner1000BasetFullDuplexCapable()))) {
                    negotiationError_ =
                              EthNegotiationErrorDuplexIncompatible;
                } else {
                    negotiationError_ =
                        EthNegotiationErrorLineSpeedIncompatible;
                }
                negotiationStatus_ = EthNegotiationStatusError;
                linkStatus_ = EthLinkStatusDown;
            }
            
            EthFlowControlModeSet * pfcms = &permittedFlowControlMode_;
            bool noPauseAllowed =
                pfcms->member( EthFlowControlModeNone );
            bool recvOnlyAllowed =
                pfcms->member( EthFlowControlModeRecvOnly );
            bool sendOnlyAllowed =
                pfcms->member( EthFlowControlModeSendOnly );
            bool sendRecvAllowed =
                pfcms->member( EthFlowControlModeSendRecv );
            
            bool noPausePossible = true;
            bool recvOnlyPossible =
                ( partner.asymmetricPauseToPartner() ||
                  partner.symmetricPause() );
            bool sendOnlyPossible =
                ( partner.asymmetricPauseToPartner() &&
                  partner.symmetricPause() );
            bool sendRecvPossible =
                ( (partner.asymmetricPauseToPartner() &&
                   partner.symmetricPause()) ||
                  partner.symmetricPause() );
            
            if( sendRecvPossible && sendRecvAllowed ) {
                flowControlMode_ = EthFlowControlModeSendRecv;
            } else if( recvOnlyPossible && recvOnlyAllowed ) {
                flowControlMode_ = EthFlowControlModeRecvOnly;
            } else if( sendOnlyPossible && sendOnlyAllowed ) {
                flowControlMode_ = EthFlowControlModeSendOnly;
                  } else if( noPausePossible && noPauseAllowed ) {
                      flowControlMode_ = EthFlowControlModeNone;
                  } else {
                      negotiationStatus_ = EthNegotiationStatusError;
                      // The only way we get here is if the other side is not
                      // capable of something I insist on using.  So, I must
                      // be insisting on using something.
                      assert( !noPauseAllowed );
                      // Figure out whether to report a send-disagreement, a
                      // receive-disagreement, or both.
                      if( sendOnlyAllowed ) {
                          if( sendRecvPossible ) {
                                // The problem would go away if the user turned
                                // on receive capability.
                              negotiationError_ =
                                  EthNegotiationErrorFlowControlRecvIncompatible;
                          } else {
                                // Any advertisement where peer offers to send
                                // can be accepted.
                              negotiationError_ =
                                    EthNegotiationErrorFlowControlSendIncompatible;
                          }
                      } else if( recvOnlyAllowed ) {
                          // Any advertisement where peer offers to receive
                          // can be accepted.
                          negotiationError_ =
                              EthNegotiationErrorFlowControlRecvIncompatible;
                      } else if( sendOnlyPossible && !recvOnlyPossible ) {
                          negotiationError_ =
                              EthNegotiationErrorFlowControlRecvIncompatible;
                      } else if( recvOnlyPossible && !sendOnlyPossible ) {
                          negotiationError_ =
                              EthNegotiationErrorFlowControlSendIncompatible;
                      } else {
                          negotiationError_ =
                              EthNegotiationErrorFlowControlBothIncompatible;
                      }
                  }
            
            loopback_ = permittedLoopback_;
            if( lineSpeed_ == EthLineSpeed1Gbps ) {
                if( status1000Baset.localTransmitterIsMaster() ) {
                    clockMode_ = EthClockModeMaster;
                } else {
                    clockMode_ = EthClockModeSlave;
                }
            } else {
                clockMode_ = EthClockModeNone;
            }
            
            remoteFaultIndication_ = (partner.remoteFault() ?
                                      EthFaultIndicationLinkFailure :
                                      EthFaultIndicationNone);
        } else {
            // Link is up but autonegotiation didn't complete ??
            // We consider the link down until negotiation is complete.
            linkStatus_ = EthLinkStatusDown;
        }
    } else {
        // maybe link isn't coming up because of some error condition.
        // we try and resolve the error and see if link comes up
        if( autonegError() ) {
            SM_throwEvent( WaitForAutonegErrorResolution );
            // state_ = StateWaitForAutonegErrorResolution;
        }
    }
}




State StateWaitForAutonegErrorResolution() functionBeforeRun() {
    SM_throwEvent( autoMakeProgress )
}

// transition for autoMakeProgress.
Transition in StateWaitForAutonegErrorResolution() {
    if( autonegError() ) {
        maybeResolveAutonegErrors();
    } else {
        SM_throwEvent( WaitAutonegFinish );
        // state_ = StateWaitAutonegFinish;
    }
}

bool
EthMiiPhyMan::autonegError() {

    switch( phyType_ ) {
      case EthMiiPhyTypeBcm5400:
        // The Brcm phy may take upto two minutes to link up.
        // See CSCdp90760 for details.
        
        // AutoNegErrors are still being resolved in 
        // maybeResolveAutoNegErrors()
        if( ( brcm5400AutoNegErrorFixState_ != 
              Brcm5400AutoNegErrorFixStateNull ) ) {
            return true;
        }
        
        HalEthBcm5400InterruptStatusReg interruptStatusReg;
        interruptStatusReg.rep( halEthMiiDevice_->readableVendorReg(
            HalEthBcm5400InterruptStatusRegNumber ) );
        if( interruptStatusReg.noHcdLink() ) {
            brcm5400AutoNegErrorFixState_ = 
                Brcm5400AutoNegErrorFixStateErrorDetected;
            return true;
        } else {
            return false;
        }
        break;
        
      case EthMiiPhyTypeBcm5208:
      case EthMiiPhyTypeBcm5218:
      case EthMiiPhyTypeLxt9782:
      case EthMiiPhyTypeMrvl88E1000:
      case EthMiiPhyTypeMrvl88E1040:
        return false;
        break;
        
      default:
        assert( 0 );
        break;
    }
    return false;
}

void
EthMiiPhyMan::maybeResolveAutonegErrors() {
    // we should come here only when an autoNegError
    // has been detected and someone is waiting for that to be 
    // resolved.
    assert( state_ = StateWaitForAutonegErrorResolution );
    
    if( halEthMiiDevice_->hardwareWriteVersion() !=
        halEthMiiDevice_->softwareWriteVersion() ) {
        return;
    }    
    
    switch( phyType_ ) {
      case EthMiiPhyTypeBcm5400: {
          switch( brcm5400AutoNegErrorFixState_ ) {
            case Brcm5400AutoNegErrorFixStateErrorDetected:
            case Brcm5400AutoNegErrorFixStateStep1:
              loopbackEnable( true );
              brcm5400AutoNegErrorFixState_ = 
                  Brcm5400AutoNegErrorFixStateStep2;
              break;
            case Brcm5400AutoNegErrorFixStateStep2:
              loopbackEnable( false );
              brcm5400AutoNegErrorFixState_ = 
                  Brcm5400AutoNegErrorFixStateStep3;
              break;
            case Brcm5400AutoNegErrorFixStateStep3:
              // Write the same values that we had written out when 
              // we tried to autonegotiate earlier
              halEthMiiDevice_->controlReg( savedControlReg_ );
              halEthMiiDevice_->adReg( savedAdReg_ );
              halEthMiiDevice_->halEth1000BasetControlReg( 
                  saved1000BasetControlReg_ );
              brcm5400AutoNegErrorFixState_ = 
                  Brcm5400AutoNegErrorFixStateErrorResolved;
              break;
            case  Brcm5400AutoNegErrorFixStateErrorResolved:
              // make sure that we reset our error state to Null as 
              // someone else checks for this
              brcm5400AutoNegErrorFixState_ = Brcm5400AutoNegErrorFixStateNull;
              brcm5400AutoNegErrorFixCount_++;
              break;
            default:
              assert( 0 );
              break;
          }
          
          break;
      }
        
      case EthMiiPhyTypeBcm5208:
      case EthMiiPhyTypeBcm5218:
      case EthMiiPhyTypeLxt9782:
      case EthMiiPhyTypeMrvl88E1000:
      case EthMiiPhyTypeMrvl88E1040:
        assert( 0 );
        break;

      default:
        assert( 0 );
        break;
    }
}

void
EthMiiPhyMan::setMiiRegDefaults() {

    // For now, the EthMiiPhyMan only supports the
    // Broadcom 5208 phy.  One could imagine reading out the
    // phyid or board id to determine which phy is plugged in.

    HalEthControlReg controlReg;
    HalEthAdReg adReg;
    HalEthStatusReg statusReg;

    controlReg.rep( 0 );
    controlReg.reset( false );
    controlReg.loopbackEnable( false );
    controlReg.lineSpeed( EthLineSpeed100Mbps );
    controlReg.autoNegotiationEnable( true );
    controlReg.powerDown( false );
    controlReg.isolate( false );
    controlReg.restartAutoNegotiation( false );
    controlReg.duplex( EthDuplexHalf );
    
    adReg.rep( 0 );
    adReg.nextPageEnable( false );
    adReg.remoteFault( false );
    adReg.base10tHalfDuplexCapable( true );
    adReg.base10tFullDuplexCapable( true );
    adReg.base100txHalfDuplexCapable( true );
    adReg.base100txFullDuplexCapable( true );
    adReg.base100t4Capable( false );
    adReg.selectorField( HalEthAdSelectorField802_3 );

    statusReg.rep( 0 );
    statusReg.mfPreambleSuppression( false );

    halEthMiiDevice_->controlReg( controlReg );
    halEthMiiDevice_->adReg( adReg );
    halEthMiiDevice_->statusReg( statusReg );
    
    switch( phyType_ ) {

      case EthMiiPhyTypeBcm5208: {
          controlReg.collisionTestEnable( false );

        // Write out the defaults for all vendor specific registers
        // This handles case where diagnostics has written something
        // strange into one of the registers.  The EthMiiPhyMan
        // controls all phy accesses (except those of the online and offline
        // diagnostics).

        // First set all vendor specific registers to 0 since most of
        // the defaults are zero.
        for( u_int regNum = HalEthMiiVendorRegNumMin;
             regNum <= HalEthMiiVendorRegNumMax;
             regNum++ ) {

            halEthMiiDevice_->writableVendorReg( regNum, 0 );
        }
        
        // Then handle registers with non-zero default values:

        HalEthBcm5208AuxControlStatusReg auxControlStatusReg;
        auxControlStatusReg.rep( 0 );
        // According to jeffh, need 4 ns rise time.  Important
        // for EMI and interoperability.
        auxControlStatusReg.edgeRate( 4 );
        halEthMiiDevice_->writableVendorReg(
            HalEthBcm5208RegNumberAuxControlStatus,
            auxControlStatusReg.rep() );
        

        HalEthBcm5208AuxMode2Reg auxMode2;
        auxMode2.rep( 0 ); 
        // The phy echos transmitted packets on the phy receive pins
        // in 10 Base-T half duplex.  Astrodome rev 1 doesn't drop
        // these packets.  Therefore, we use the phy's ability to _not_
        // echo the transmitted packets.  The rev 2 Astrodome may
        // provide a different solution to this problem.
        // NOTE: This capability only exists for rev 2 and greater
        // of the Bcm5208
        auxMode2.block10BasetEchoMode( true );

        // Since 10/100 phys are capable of detecting non-negotaiting
        // links in parallel with autonegotiation, disallow link coming
        // up when the detected speed doesn't match the speed allowed in
        // the local advertisement reg.
        auxMode2.qualParallelDetectModeEnable( true );
        halEthMiiDevice_->writableVendorReg( HalEthBcm5208RegNumberAuxMode2,
                                             auxMode2.rep() );


        // Per Bcm5208 documentation,
        halEthMiiDevice_->writableVendorReg( HalEthBcm5208RegNumberInterrupt,
                                             0xf00 );
        
        break;
      }

      case EthMiiPhyTypeBcm5218: {
          // Write out the defaults for all vendor specific registers.
          for( u_int regNum = HalEthMiiVendorRegNumMin;
               regNum <= HalEthMiiVendorRegNumMax;
               regNum++ ) {
              halEthMiiDevice_->writableVendorReg( regNum, 0 );
          }
          // Now let us write out registers with non-zero default values.
          // Look at haleth/Types.h as to what all code we reuse from
          // Bcm5208.
          HalEthBcm5208AuxControlStatusReg auxControlStatusReg;
          auxControlStatusReg.rep( 0 );

          // According to jeffh, need 4 ns rise time.  Important
          // for EMI and interoperability.
          auxControlStatusReg.edgeRate( 4 );
          halEthMiiDevice_->writableVendorReg(
              HalEthBcm5218RegNumberAuxControlStatus,
              auxControlStatusReg.rep() );
        

          HalEthBcm5218AuxMode2Reg auxMode2;
          // Don't set the value on Bcm5218's AuxMode2 register to zero.
          // The constructor for that register set's it to appropriate
          // value that needs to be written for some reserved bits.
          // auxMode2.rep( 0 ); 

          // The phy echos transmitted packets on the phy receive pins
          // in 10 Base-T half duplex.  Astrodome rev 1 doesn't drop
          // these packets.  Therefore, we use the phy's ability to _not_
          // echo the transmitted packets.  The rev 2 Astrodome may
          // provide a different solution to this problem.
          // NOTE: This capability only exists for rev 2 and greater
          // of the Bcm5208
          auxMode2.defaultInit();
          auxMode2.block10BasetEchoMode( true );
          // Since 10/100 phys are capable of detecting non-negotaiting
          // links in parallel with autonegotiation, disallow link coming
          // up when the detected speed doesn't match the speed allowed in
          // the local advertisement reg.
          auxMode2.qualParallelDetectModeEnable( true );

          // Make sure that the Auxiliary Mode 2 register is set to its
          // default.  If not, we might mess up with the output of LINKLED
          // pin on phy.  This can indirectly effect the operation of
          // santana fpga on Veo.  For example, if the serial LED mode
          // is enabled, we see a series of pulses on the LINKLED output
          // for port 2(first port numbered 1) which will result in power
          // being shut off to port 2 by santana when the pulse goes down.
          // - pjonnala
          assert( auxMode2.rep() == HalEthBcm5218AuxMode2RegDefault );

          halEthMiiDevice_->writableVendorReg( HalEthBcm5218RegNumberAuxMode2,
                                               auxMode2.rep() );

          // Per Bcm5218 documentation,
          halEthMiiDevice_->writableVendorReg( HalEthBcm5218RegNumberInterrupt,
                                               0xf00 );

          HalEthAnNextPageTransmitReg reg;
          reg.rep( 0x2001 );
          halEthMiiDevice_->anNextPageTransmitReg( reg );
          break;
      }

      case EthMiiPhyTypeBcm5400: {

        // The Phy is reset before reaching this code which clears out any
        // state left by diagnostics.

        // Handle registers with non-zero default values:
        HalEthBcm5400ExtendedControlReg extendedControlReg;
        extendedControlReg.rep( 0x8002 );
        halEthMiiDevice_->writableVendorReg(
            HalEthBcm5400RegNumberExtendedControl, extendedControlReg.rep() );
        break;
      }
      case EthMiiPhyTypeLxt9782: {
        HalEthLxt9782TxControlOneReg txControlReg;
        txControlReg.rep( 0 );
        txControlReg.riseTime( 4 );
        halEthMiiDevice_->writableVendorReg(
            HalEthLxt9782RegNumberTxControlOne,
            txControlReg.rep() );
        // this register and value are given by jeffh and not documented 
        // in Lxt9782 spec. ( April 2000 version ). we don't know what's 
        // the function of the register and just blindly set the given 
        // value. This value (0x0008) was to let linkup when port connected
        // to short cable. 
        halEthMiiDevice_->writableVendorReg(
            HalEthLxt9782RegNumberVendorReg27,
            0x0008 );

        break;
      }
      case EthMiiPhyTypeMrvl88E1000: 
      case EthMiiPhyTypeMrvl88E1040: {
          controlReg.restartAutoNegotiation( true );
          controlReg.reset( true );
          halEthMiiDevice_->controlReg( controlReg );
          break;
      }

      case EthMiiPhyTypeNull:
      default:
        assert( 0 );
        // Currently unsupported type.
        break;
    }
}    

void
EthMiiPhyMan::configurePhyConnector() {

    switch( phyType_ ) {

      case EthMiiPhyTypeBcm5208: 
      case EthMiiPhyTypeBcm5218: {

        switch( connectorType_ ) {

          case EthConnectorType100MtrjShortwave: {
            
            // A 100 FX port uses far end fault transmission
            // to link with its partner, so we need to enable
            // this feature per spec for FX port operation.
            
            // Based on empirical data, 100 FX also need scrambling disabled,
            // cim disabled,  and baseline wander correction disabled
            HalEthAuxControlReg auxControl;
            auxControl.rep( halEthMiiDevice_->writableVendorReg(
                HalEthBcm5208RegNumberAuxControl ) );
            auxControl.cimDisable( true );
            auxControl.scramblerDisable( true );
            auxControl.baselineWanderCorrecterDisable( true );
            auxControl.farEndFaultDetectorEnable( true );
            halEthMiiDevice_->writableVendorReg(
                HalEthBcm5208RegNumberAuxControl,
                auxControl.rep() );
            break;
          }
          default:
            // No special configuration needed for any other connector type.
            break;
        }
        break;
      }
        
      case EthMiiPhyTypeBcm5400: {
        // No special configuration needed for any connector type.
        break;
      }
      case EthMiiPhyTypeLxt9782: {
          // No special configuration needed for any connector type.
          break;
      }
      case EthMiiPhyTypeMrvl88E1000: {
          // No special configuration needed for any connector type.
          break;
      }
      case EthMiiPhyTypeMrvl88E1040: {
          // No special configuration needed for any connector type.
          break;
      }
      default:
        assert( 0 );
        // Currently unsupported type.
        break;
    }
}

void
EthMiiPhyMan::disablePhyTransmission( bool disable ) {

    switch( phyType_ ) {
        
      case EthMiiPhyTypeBcm5208: {
        // transmitDisable appears to work only in 100 forced.
        if( disable ) {
            HalEthControlReg control;
            control.rep( 0 );
            control.lineSpeed( EthLineSpeed100Mbps );
            halEthMiiDevice_->controlReg( control );
        }
        
        HalEthAuxControlReg auxControl;
        auxControl.rep( halEthMiiDevice_->writableVendorReg(
            HalEthBcm5208RegNumberAuxControl ) );
        auxControl.transmitDisable( disable );
        halEthMiiDevice_->writableVendorReg( HalEthBcm5208RegNumberAuxControl,
                                             auxControl.rep() );
      }
        break;
      case EthMiiPhyTypeBcm5218: {
        // transmitDisable appears to work only in 100 forced
        if( disable ) {
            HalEthControlReg control;
            control.rep( 0 );
            control.lineSpeed( EthLineSpeed100Mbps );
            halEthMiiDevice_->controlReg( control );
        }
        
        HalEthBcm5218AuxControlReg auxControl;
          auxControl.rep( halEthMiiDevice_->writableVendorReg(
              HalEthBcm5218RegNumberAuxControl ) );
          auxControl.transmitDisable( disable );
          halEthMiiDevice_->writableVendorReg( 
              HalEthBcm5218RegNumberAuxControl,
              auxControl.rep() );
          break;
      }
      case EthMiiPhyTypeBcm5400: {
        HalEthBcm5400ExtendedControlReg controlReg;
        controlReg.rep( halEthMiiDevice_->writableVendorReg(
            HalEthBcm5400RegNumberExtendedControl ) );
        controlReg.transmitDisable( disable );
        halEthMiiDevice_->writableVendorReg(
            HalEthBcm5400RegNumberExtendedControl,
            controlReg.rep() );
        break;
      }
      case EthMiiPhyTypeLxt9782: {
          HalEthLxt9782PortConfigReg portConfigReg;
          portConfigReg.rep( halEthMiiDevice_->writableVendorReg(
              HalEthLxt9782RegNumberPortConfig ) );
          portConfigReg.transmitDisable( disable );
          halEthMiiDevice_->writableVendorReg(
              HalEthLxt9782RegNumberPortConfig,
              portConfigReg.rep() );
          break;
      } 
      case EthMiiPhyTypeMrvl88E1000: 
      case EthMiiPhyTypeMrvl88E1040: {
          if( disable ) {
              HalEthControlReg control;
              control.rep( 0 );
              control.loopbackEnable( false );
              control.powerDown( true );
              halEthMiiDevice_->controlReg( control );
          } else {
              HalEthControlReg control;
              control.rep( 0 );
              control.powerDown( false );
              control.reset( true );
          }
             
          break;
      }
      case EthMiiPhyTypeNull:
      default:
        assert( 0 );
        // Currently unsupported type.
        break;
    }
}        

u_int
EthMiiPhyMan::disconnectCounter() {

    u_int disconnects = 0;
    
    switch( phyType_ ) {

      case EthMiiPhyTypeBcm5208: {

        disconnects =
            halEthMiiDevice_->readableVendorReg(
                HalEthBcm5208RegNumberDisconnectCounter );
        break;
      }
      case EthMiiPhyTypeBcm5218: {
          // We don't have any disconnect counters available
          // for us on this phy.  We'll just return 0.
          disconnects = 0;
          break;
      }
      case EthMiiPhyTypeBcm5400: {
        // XXX It turns out that our disconnect counter code is logically wrong
        // XXX and that we don't need it.  Bug CSCdp21270 is open to track this
        // XXX issue.
        break;
      }
      case EthMiiPhyTypeLxt9782: {
          // no register field in LXT9782 tells disconnect counter
          // assume default 0.
          break;
      }
      case EthMiiPhyTypeMrvl88E1000: {
          // no disconnect counter on this phy. Also, we shouldn't be using 
          // it here. Look at CSCdp21270.
          break;
      }
      case EthMiiPhyTypeMrvl88E1040: {
          // no disconnect counter on this phy. Also, we shouldn't be using 
          // it here. Look at CSCdp21270.
          break;
      }
      case EthMiiPhyTypeNull:
      default:
        assert( 0 );
        // Currently unsupported type.
        break;
    }

    return disconnects;
}

void
EthMiiPhyMan::ethMiiPhyType( EthMiiPhyType type ) {
    assert( type != EthMiiPhyTypeNull &&
            phyType_ == EthMiiPhyTypeNull );
    phyType_ = type;
}

EthMiiPhyType
EthMiiPhyMan::ethMiiPhyType() {
    return phyType_;
}

void
EthMiiPhyMan::restartPhyInitStateMachine() {

    switch( phyType_ ) {

      default:
        assert( 0 );
        
      case EthMiiPhyTypeBcm5218: 
      case EthMiiPhyTypeLxt9782:
      case EthMiiPhyTypeMrvl88E1000: 
      case EthMiiPhyTypeMrvl88E1040: {
        setMiiRegDefaults();
        phyInitState_ = PhyInitStateComplete;
        break;
      }

      case EthMiiPhyTypeBcm5208: 
      case EthMiiPhyTypeBcm5400: {
        // We should soft reset the phy when we first bring them up
        // as it could be in some bizzare state after online diags 
        // has been run or the phy may not have come out of hard reset 
        // cleanly.
        HalEthControlReg controlReg;
        controlReg.rep( 0 );
        controlReg.reset( true );
        halEthMiiDevice_->controlReg( controlReg );
        phyInitState_ = PhyInitStateWaitForPhyReset;
        break;
      }
    }
}
              
void
EthMiiPhyMan::maybeMakePhyInitProgress() {

    PhyInitState lastPhyInitState;
            
    do {

        lastPhyInitState = phyInitState_;

        switch( phyInitState_ ) {

          default:
            assert( 0 );
            
          case PhyInitStateWaitForPhyReset: {

            // We are waiting for the reset to get pushed out to the MII
            // device.  See if it has happened.
            if( halEthMiiDevice_->hardwareWriteVersion() !=
                halEthMiiDevice_->softwareWriteVersion() ) {
                break;
            }

            // Then it would be good to wait until we've at least read our
            // current state once since the reset.
            if( halEthMiiDevice_->hardwareReadVersion() !=
                halEthMiiDevice_->softwareWriteVersion() ) {
                break;
            }

            switch( phyType_ ) {
                
              default:
                assert( 0 );
                break;
                
              case EthMiiPhyTypeBcm5208:
                setMiiRegDefaults();
                phyInitState_ = PhyInitStateComplete;
                break;
                
              case EthMiiPhyTypeBcm5400:
                // We have to write a sequence of vendor specific registers and
                // values.  We use a table driven approach.
                setMiiRegDefaults();
                bcm5400PowerFixStep_ = 0;
                phyInitState_ = PhyInitStateWaitBcm5400PowerFix;
                break;
                
              case EthMiiPhyTypeBcm5218:
              case EthMiiPhyTypeLxt9782:
              case EthMiiPhyTypeMrvl88E1000:
              case EthMiiPhyTypeMrvl88E1040:
                assert( 0 );
                phyInitState_ = PhyInitStateComplete;
                break;
            }
                
            break;
          }

          case PhyInitStateWaitBcm5400PowerFix: {

            if( halEthMiiDevice_->hardwareWriteVersion() !=
                halEthMiiDevice_->softwareWriteVersion() ) {
                break;
            }

            if( bcm5400PowerFixStep_ < Bcm5400PowerFixMaxSteps ) {
                StateStep nextStep =
                    bcm5400PowerFixStepTable_[ bcm5400PowerFixStep_ ];
                halEthMiiDevice_->writableVendorReg( nextStep.regNum_,
                                                     nextStep.val_ );
                bcm5400PowerFixStep_++;
            } else {
                phyInitState_ = PhyInitStateComplete;
            }
                    
            break;
          }

          case PhyInitStateComplete: {

            needToInitializePhy_ = false;
            lastCompleteTime_ = MicrosecondsNever;
            restartStateMachine();
            break;
          }
        }
    } while( phyInitState_ != lastPhyInitState );
}


void
EthMiiPhyMan::loopbackEnable( bool lb )
{
    HalEthControlReg controlReg;
    controlReg.rep( loopbackEnableControlRegRep( lb ) );

    if( lb ) {
        // On most Phys, this is enough to enable/disable loopback.
        // But the Marvell Phys on Spacely need to do more work.
        
        loopbackState_ = EthMiiPhyLoopbackStateEnabled;
        
        switch( phyType_ ) {

          case EthMiiPhyTypeBcm5218:
          case EthMiiPhyTypeBcm5208:
          case EthMiiPhyTypeLxt9782:
          case EthMiiPhyTypeBcm5400: 
          case EthMiiPhyTypeMrvl88E1040: {
              break;
          }
          case EthMiiPhyTypeMrvl88E1000: {
              controlReg.lineSpeed( EthLineSpeed1Gbps );
              loopbackState_ = EthMiiPhyLoopbackStateInProgress;
              mrvlLoopbackState_ = Mrvl88E1000LoopbackEnableStarted;
              break;
          }
          default: {
              assert( 0 );
              break;
          }
        }
        halEthMiiDevice_->controlReg( controlReg );
                
        HalEthStatusReg statusReg;
        statusReg.rep( 0 );
        statusReg.mfPreambleSuppression( false );  // only writable bit in reg
        
        halEthMiiDevice_->statusReg( statusReg );
        
        if( phyType_ == EthMiiPhyTypeBcm5400 ||
            phyType_ == EthMiiPhyTypeMrvl88E1000 ||
            phyType_ == EthMiiPhyTypeMrvl88E1040 ) {
            return;
        }

        // First set all vendor specific registers to 0 since most of
        // the defaults are zero.
        
        for( u_int regNum = HalEthMiiVendorRegNumMin;
             regNum <= HalEthMiiVendorRegNumMax;
             regNum++ ) {

            halEthMiiDevice_->writableVendorReg( regNum, 
                                                 defaultVendorRegValue( 
                                                     regNum ) );
        }
        
    } else {
        // This is a workaround for a bug
        // in the Broadcom phy.  It has a problem with 100 Mbps loopback,
        // and the phy needs to be reset to get it out of loopback.
        
        loopbackState_ = EthMiiPhyLoopbackStateDisabled;
        switch( phyType_ ) {
          case EthMiiPhyTypeBcm5208:
          case EthMiiPhyTypeBcm5218: 
          case EthMiiPhyTypeBcm5400:
          case EthMiiPhyTypeLxt9782: 
          case EthMiiPhyTypeMrvl88E1040: {
              break;
          }
          case EthMiiPhyTypeMrvl88E1000: {
              loopbackState_ = EthMiiPhyLoopbackStateDisableInProgress;
              mrvlLoopbackState_ = Mrvl88E1000LoopbackDisableStarted;
              break;
          }
          default: {
              assert( 0 );
              break;
          }
        }

        // This is a workaround for a bug in the Broadcom 52x8 phys.
        // Although the documentation says that simply resetting
        // the phy will restore the link integrity test, empirical
        // data indicates that doing so without explicitly reenabling
        // the test makes it impossible to disable it in the future
        // (e.g. the next time we want to go into loopback).  So
        // we explicitly reenable the link integrity test when disabling
        // loopback.  See CSCdt95608.
        if( phyType_ == EthMiiPhyTypeBcm5208 ||
            phyType_ == EthMiiPhyTypeBcm5218 ) {
            HalEthBcm5208AuxControlStatusReg auxControlStatusReg;
            auxControlStatusReg.rep( 0 );
            // According to jeffh, need 4 ns rise time.  Important
            // for EMI and interoperability.
            auxControlStatusReg.edgeRate( 4 );
            halEthMiiDevice_->writableVendorReg(
                HalEthBcm5208RegNumberAuxControlStatus,
                auxControlStatusReg.rep() );
        }

        halEthMiiDevice_->controlReg( controlReg );
    }
}

// This function fills in the control register values necessary to
// put the phy into or out of loopback and returns the representation.
u_int16
EthMiiPhyMan::loopbackEnableControlRegRep( bool lb ) {
    HalEthControlReg controlReg;

    controlReg.rep( 0 );

    if( lb ) {
        // Loopback for the Broadcom phy only works for forced 100 Mb.  In
        // order to put a port in loopback, we put the port in forced 100 Mb
        // Full duplex mode, disable transmission in vendor reg 0x10 (100Base-x
        // Aux control -- 0x2000) and turn off the link integrity test (reg
        // 0x18, Auxiliary Control/Status -- 0x4000) per the errata notes.
        
        // We need to write ALL of the writable mii registers because no one
        // guarantees what state they're in at the point we go to put our ports
        // in loopback.

        controlReg.reset( false );
        controlReg.loopbackEnable( true );
        controlReg.lineSpeed( EthLineSpeed100Mbps );
        controlReg.autoNegotiationEnable( false );
        controlReg.powerDown( false );
        controlReg.isolate( false );
        controlReg.restartAutoNegotiation( false );
        controlReg.duplex( EthDuplexFull );

        // Collision test feature is available only in
        // 5208 and Lxt9782.  It is not present in 5218.

        switch( phyType_ ) {

          case EthMiiPhyTypeBcm5218: {
              break;
          }
          case EthMiiPhyTypeBcm5208:
          case EthMiiPhyTypeLxt9782: {
              controlReg.collisionTestEnable( false );
              break;
          }
          case EthMiiPhyTypeBcm5400: {
              controlReg.lineSpeed( EthLineSpeed1Gbps );
              controlReg.collisionTestEnable( false );
              break;
          }
          case EthMiiPhyTypeMrvl88E1000: 
          case EthMiiPhyTypeMrvl88E1040: {
              controlReg.lineSpeed( EthLineSpeed1Gbps );
              break;
          }
          default: {
              assert( 0 );
              break;
          }
        }
    } else {
        // This is a workaround for a bug
        // in the Broadcom phy.  It has a problem with 100 Mbps loopback,
        // and the phy needs to be reset to get it out of loopback.

        switch( phyType_ ) {
          case EthMiiPhyTypeBcm5218:
          case EthMiiPhyTypeBcm5208: {
              controlReg.reset( true );
              break;
          }
          case EthMiiPhyTypeBcm5400: {
              controlReg.reset( false );
              controlReg.loopbackEnable( false );
              controlReg.autoNegotiationEnable( true );
              controlReg.powerDown( false );
              controlReg.isolate( false );
              controlReg.restartAutoNegotiation( false );
              controlReg.duplex( EthDuplexHalf );
              break;
          }
          case EthMiiPhyTypeLxt9782: 
          case EthMiiPhyTypeMrvl88E1040: {
              controlReg.reset( false );
              controlReg.loopbackEnable( false );
              controlReg.autoNegotiationEnable( true );
              controlReg.powerDown( false );
              controlReg.isolate( false );
              controlReg.restartAutoNegotiation( false );
              controlReg.duplex( EthDuplexHalf );
              break;
          }
          case EthMiiPhyTypeMrvl88E1000: {
              loopbackState_ = EthMiiPhyLoopbackStateDisableInProgress;
              mrvlLoopbackState_ = Mrvl88E1000LoopbackDisableStarted;
              // no register settings; the state machine will handle it.
              break;
          }
          default: {
              assert( 0 );
              break;
          }
        }
    }

    return controlReg.rep();
}

u_int16
EthMiiPhyMan::defaultVendorRegValue( HalEthMiiRegNum miiRegNum ) const {
    u_int16 regValue = 0;
    switch( phyType_ ) {
      case EthMiiPhyTypeBcm5208: {
          regValue = defaultBcm5208VendorRegValue( miiRegNum );
          break;
      }
      case EthMiiPhyTypeBcm5218: {
          regValue = defaultBcm5218VendorRegValue( miiRegNum );
          break;
      }
      case EthMiiPhyTypeLxt9782: {
          regValue = defaultLxt9782VendorRegValue( miiRegNum );
          break;
      }
      case EthMiiPhyTypeMrvl88E1000: {
          regValue = defaultMrvl88E1000VendorRegValue( miiRegNum );
          break;
      }
      case EthMiiPhyTypeMrvl88E1040: {
          regValue = defaultMrvl88E1040VendorRegValue( miiRegNum );
          break;
      }
      case EthMiiPhyTypeBcm5400: {
          // use default value: 0
          break;
      }
      default: {
          assert( 0 );
          break;
      }
    }
    return regValue;
}

u_int16
EthMiiPhyMan::defaultBcm5208VendorRegValue( HalEthMiiRegNum miiRegNum ) const {
    assert( miiRegNum >= HalEthMiiVendorRegNumMin &&
            miiRegNum <= HalEthMiiVendorRegNumMax );
    u_int16 regVal = 0;
    switch( miiRegNum ) {
      case HalEthBcm5208RegNumberAuxControl: {
          HalEthAuxControlReg auxControlReg;
          auxControlReg.rep( 0 );
          auxControlReg.transmitDisable( true );
          regVal = auxControlReg.rep();
          break;
      }
      case HalEthBcm5208RegNumberAuxControlStatus: {
          HalEthBcm5208AuxControlStatusReg auxControlStatusReg;
          auxControlStatusReg.rep( 0 );
          auxControlStatusReg.linkDisable( true );
          // According to jeffh, need 4 ns rise time.  Important
          // for EMI and interoperability.
          auxControlStatusReg.edgeRate( 4 );   
          regVal = auxControlStatusReg.rep();
          break;
      }
      default: {
          // Nothing, we just return zero for other registers.
          break;
      }
    }
    return regVal;
}
    
u_int16
EthMiiPhyMan::defaultBcm5218VendorRegValue( HalEthMiiRegNum miiRegNum ) const {
    assert( miiRegNum >= HalEthMiiVendorRegNumMin &&
        miiRegNum <= HalEthMiiVendorRegNumMax );
    u_int16 regVal = 0;
    switch( miiRegNum ) {
      case HalEthBcm5218RegNumberAuxControl: {
          HalEthBcm5218AuxControlReg auxControlReg;
          auxControlReg.rep( 0 );
          auxControlReg.transmitDisable( true );
          regVal = auxControlReg.rep();
          break;
      }
      case HalEthBcm5218RegNumberAuxControlStatus: {
          HalEthBcm5208AuxControlStatusReg auxControlStatusReg;
          auxControlStatusReg.rep( 0 );
          auxControlStatusReg.linkDisable( true );
          // According to jeffh, need 4 ns rise time.  Important
          // for EMI and interoperability.
          auxControlStatusReg.edgeRate( 4 );   
          regVal = auxControlStatusReg.rep();
          break;
      }
      default: {
          // Nothing, we just return zero for other registers.
          break;
      }
    }
    return regVal;
}
              
u_int16
EthMiiPhyMan::defaultLxt9782VendorRegValue( HalEthMiiRegNum miiRegNum ) const {
    assert( miiRegNum >= HalEthMiiVendorRegNumMin &&
            miiRegNum <= HalEthMiiVendorRegNumMax );
    u_int16 regVal = 0;

    switch( miiRegNum ) {
      case HalEthLxt9782RegNumberTxControlOne: {
          HalEthLxt9782TxControlOneReg txControlReg;
          txControlReg.rep( 0 );
          txControlReg.riseTime( 4 );
          regVal = txControlReg.rep();
          break;
      }
      default: {
          // Nothing, we just return zero for other registers.
          break;
      }
    }
    return regVal;    
}

u_int16
EthMiiPhyMan::defaultMrvl88E1000VendorRegValue( HalEthMiiRegNum miiRegNum ) 
    const {
    assert( miiRegNum >= HalEthMiiVendorRegNumMin &&
            miiRegNum <= HalEthMiiVendorRegNumMax );

    u_int16 regVal = 0;

    switch( miiRegNum ) {
        
      default: {
          // Nothing, we just return zero for other registers.
          break;
      }
    }
    return regVal;    


}

u_int16
EthMiiPhyMan::defaultMrvl88E1040VendorRegValue( HalEthMiiRegNum miiRegNum ) 
    const {
    assert( miiRegNum >= HalEthMiiVendorRegNumMin &&
            miiRegNum <= HalEthMiiVendorRegNumMax );

    u_int16 regVal = 0;

    switch( miiRegNum ) {
        
      default: {
          // Nothing, we just return zero for other registers.
          break;
      }
    }
    return regVal;    

}

void
EthMiiPhyMan::forcedDpmDeviceDiscoveryModeEnable( 
    EthDpmDeviceDiscoveryModeEnable mode ) {
    assert( mode == EthDpmDeviceDiscoveryModeEnableOn ||
            mode == EthDpmDeviceDiscoveryModeEnableOff );
    assert( dpmDeviceDiscoveryCapable() );
    switch( phyType_ ) {
      case EthMiiPhyTypeBcm5218: {
          HalEthBcm5218EnableDpmStatReg ena;
          ena.rep( 0 );
          ena.spareControlEnable( 
              mode == EthDpmDeviceDiscoveryModeEnableOn );

          halEthMiiDevice_->writableVendorReg(
              HalEthBcm5218RegNumberEnableDpmStatusReg, ena.rep());
          break;
      }
      case EthMiiPhyTypeBcm5400:
      case EthMiiPhyTypeLxt9782:
      case EthMiiPhyTypeBcm5208:
      case EthMiiPhyTypeMrvl88E1000:
      case EthMiiPhyTypeMrvl88E1040:

      default: {
          assert( 0 );
          break;
      }
    }
}

void
EthMiiPhyMan::forcedDpmDeviceDiscovery( 
    EthDpmDeviceDiscovery discovery, bool restartAutoNegotiation ) {
    assert( dpmDeviceDiscoveryCapable() );
    assert( discovery == EthDpmDeviceDiscoveryOff ||
            discovery == EthDpmDeviceDiscoveryOn );
    switch( phyType_ ) {
      case EthMiiPhyTypeBcm5218: {

          HalEthBcm5218DpmReg dpmReg;
          dpmReg.rep( 0 );
          dpmReg.dpmDetectionEnable( 
              discovery == EthDpmDeviceDiscoveryOn );
          dpmReg.restartAutoNeg( restartAutoNegotiation );
          dpmReg.transmitLinkPulseWidth(
              HalEthLinkPulseWidth150ns );
          
          HalEthExtendedStatusReg reg;
          reg.rep( dpmReg.rep());
          
          halEthMiiDevice_->extendedStatusReg( reg );
          if( restartAutoNegotiation ) {
              // We restart auto-neg no matter whether parameters
              // changed or not.
              assert( !needToInitializePhy_ );
              lastCompleteTime_ = MicrosecondsNever;
              restartStateMachine();
          }
          break;
      }
      case EthMiiPhyTypeBcm5208:
      case EthMiiPhyTypeBcm5400:
      case EthMiiPhyTypeMrvl88E1000:
      case EthMiiPhyTypeMrvl88E1040:
      default: {
          assert( 0 );
          break;
      }
    }
}


// This is needed specifically for Mrvl88E1000 Phys
// to workaround some hardware bugs
bool
EthMiiPhyMan::needToPushMoreDataToForceSpeedAndDuplex() {
    assert( permittedLoopback_ != EthLoopbackOn );
    
    if( !needToPushMoreData_ ) {
        return false;
    }
    
    if( halEthMiiDevice_->hardwareWriteVersion() !=
        halEthMiiDevice_->softwareWriteVersion() ) {
        // Not yet.
        return true;  
    }
    
    halEthMiiDevice_->controlReg( savedControlReg_ );
    needToPushMoreData_ = false;

    return needToPushMoreData_;
}


EthMiiPhyLoopbackState
EthMiiPhyMan::loopbackState() {

    switch( loopbackState_ ) {
      case EthMiiPhyLoopbackStateEnabled :
      case EthMiiPhyLoopbackStateDisabled :
        break;
        
      case EthMiiPhyLoopbackStateInProgress :
      case EthMiiPhyLoopbackStateDisableInProgress :
        makeLoopbackProgress();
        break;

      default:
        assert( 0 );
    }

    return loopbackState_;
}

void
EthMiiPhyMan::makeLoopbackProgress() {

    if( halEthMiiDevice_->hardwareWriteVersion() !=
        halEthMiiDevice_->softwareWriteVersion() ) {
        return;
    }

    HalEthControlReg controlReg;
    HalEth1000BasetControlReg base1000tControl;
    switch( phyType_ ) {

      case EthMiiPhyTypeMrvl88E1000: {
          // To put a Mrvl88E1000 Phy into loopback or to bring
          // it out of loopback, the following steps are needed.
          // This phy can do loopback only at 1Gbps (without 
          // the need of a link partner).
          // This is required as some writes to the phy require 
          // a soft reset of the phy for the write to take effect.
          // Also, this method disables the receiver on the phy
          // when putting it into loopback and avoids any side effects
          // due to a change in state of the link partner. The 
          // receiver is re-enabled when we bring the phy out of 
          // loopback.
        
          if( loopbackState_ == EthMiiPhyLoopbackStateInProgress ) {
              
              switch( mrvlLoopbackState_ ) {
                case Mrvl88E1000LoopbackEnableStarted :
                case Mrvl88E1000LoopbackEnableStateStep1 :
                  halEthMiiDevice_->writableVendorReg( 
                      HalEthMrvl88E1000PhySpecificCtrlRegNum, 0x0818 );
                  mrvlLoopbackState_ = Mrvl88E1000LoopbackEnableStateStep2;
                  break;
                  
                case Mrvl88E1000LoopbackEnableStateStep2 :
                  controlReg.rep( 0x9140 );
                  halEthMiiDevice_->controlReg( controlReg );
                  mrvlLoopbackState_ = Mrvl88E1000LoopbackEnableStateStep3;
                  break;
                  
                case Mrvl88E1000LoopbackEnableStateStep3 :
                  controlReg.rep( 0x8100 );
                  halEthMiiDevice_->controlReg( controlReg );
                  mrvlLoopbackState_ = Mrvl88E1000LoopbackEnableStateStep4;
                  break;
                  
                case Mrvl88E1000LoopbackEnableStateStep4 :
                  halEthMiiDevice_->writableVendorReg( 29, 0x001f );
                  halEthMiiDevice_->writableVendorReg( 30, 0x8ffc );
                  base1000tControl.rep( 0x0200 );
                  halEthMiiDevice_->halEth1000BasetControlReg( 
                      base1000tControl );
                  mrvlLoopbackState_ = Mrvl88E1000LoopbackEnableStateStep5;
                  break;
                  
                case Mrvl88E1000LoopbackEnableStateStep5 :
                  halEthMiiDevice_->writableVendorReg( 29, 0x001a );
                  halEthMiiDevice_->writableVendorReg( 30, 0x8ff0 );
                  mrvlLoopbackState_ = Mrvl88E1000LoopbackEnableStateStep6;
                  break;
                  
                case Mrvl88E1000LoopbackEnableStateStep6 :
                  controlReg.rep( 0x4100 );
                  halEthMiiDevice_->controlReg( controlReg );
                  mrvlLoopbackState_ = Mrvl88E1000LoopbackEnableStateComplete;
                  break;
                case Mrvl88E1000LoopbackEnableStateComplete :
                  loopbackState_ = EthMiiPhyLoopbackStateEnabled;
                  break;
                  
                default:
                  assert( 0 );
              }
              
          } else if( loopbackState_ == 
                     EthMiiPhyLoopbackStateDisableInProgress ) {
              
              switch( mrvlLoopbackState_ ) {
                case Mrvl88E1000LoopbackDisableStarted :
                case Mrvl88E1000LoopbackDisableStateStep1 :
                  halEthMiiDevice_->writableVendorReg( 29, 0x001f );
                  halEthMiiDevice_->writableVendorReg( 30, 0x0000 );
                  mrvlLoopbackState_ = Mrvl88E1000LoopbackDisableStateStep2;
                  break;
                case Mrvl88E1000LoopbackDisableStateStep2 :
                  halEthMiiDevice_->writableVendorReg( 29, 0x001a );
                  halEthMiiDevice_->writableVendorReg( 30, 0x0000 );
                  mrvlLoopbackState_ = Mrvl88E1000LoopbackDisableStateStep3;
                  break;
                case Mrvl88E1000LoopbackDisableStateStep3 :
                  controlReg.rep( 0x8000 );
                  halEthMiiDevice_->controlReg( controlReg );
                  mrvlLoopbackState_ = Mrvl88E1000LoopbackDisableStateComplete;
                  break;
                case Mrvl88E1000LoopbackDisableStateComplete :
                  loopbackState_ = EthMiiPhyLoopbackStateDisabled;
                  break;
                default:
                  assert( 0 );
              }
          }
       
          break;
      } //case EthMiiPhyTypeMrvl88E1000:

      default:
        assert( 0 );
    } // switch ( phyType_ )
}

void
EthMiiPhyMan::onNewConfiguration() {
    // Someone is telling us that the configuration has changed.
    // We'll have to invalidate out operating state.
    negotiationStatus_ = EthNegotiationStatusNotStarted;
    negotiationError_ = EthNegotiationErrorNull;
    remoteFaultIndication_ = EthFaultIndicationNull;
    linkStatus_ = EthLinkStatusNull;
    lineSpeed_ = EthLineSpeedNull;
    duplex_ = EthDuplexNull;
    loopback_ = EthLoopbackNull;
    flowControlMode_ = EthFlowControlModeNull;
}
    
EthDpmDeviceDiscoveryModeEnable
EthMiiPhyMan::dpmDeviceDiscoveryModeEnable() {
    assert( dpmDeviceDiscoveryCapable() );
    assert( halEthMiiDevice_ );
    if( halEthMiiDevice_->hardwareWriteVersion() !=
        halEthMiiDevice_->softwareWriteVersion() ) {
        // Not yet.
        return EthDpmDeviceDiscoveryModeEnableNull;
    }
    EthDpmDeviceDiscoveryModeEnable mode = 
        EthDpmDeviceDiscoveryModeEnableNull;
    switch( phyType_ ) {
      case EthMiiPhyTypeBcm5218: {
          HalEthBcm5218EnableDpmStatReg ena;
          ena.rep( halEthMiiDevice_->readableVendorReg(
              HalEthBcm5218RegNumberEnableDpmStatusReg ));
          if( ena.spareControlEnable()) {
              mode = EthDpmDeviceDiscoveryModeEnableOn;
          } else {
              mode = EthDpmDeviceDiscoveryModeEnableOff;
          }
          break;
      }
      case EthMiiPhyTypeBcm5400:
      case EthMiiPhyTypeBcm5208:
      case EthMiiPhyTypeLxt9782:
      case EthMiiPhyTypeMrvl88E1000:
      case EthMiiPhyTypeMrvl88E1040:
      default: {
          assert( 0 );
          break;
      }
    }
    return mode;
}

EthDpmDeviceDiscovery
EthMiiPhyMan::dpmDeviceDiscovery() {
    assert( dpmDeviceDiscoveryCapable() );
    assert( halEthMiiDevice_ );
    if( halEthMiiDevice_->hardwareWriteVersion() !=
        halEthMiiDevice_->softwareWriteVersion() ) {
        // Not yet.
        return EthDpmDeviceDiscoveryNull;
    }
    EthDpmDeviceDiscovery discovery = EthDpmDeviceDiscoveryNull;

    switch( phyType_ ) {
      case EthMiiPhyTypeBcm5218: {
          HalEthExtendedStatusReg ext;
          ext.rep( ( halEthMiiDevice_->extendedStatusReg()).rep());
          HalEthBcm5218DpmReg reg;
          reg.rep( ext.rep());
          if( reg.dpmDetectionEnable()) {
              discovery = EthDpmDeviceDiscoveryOn;
          } else {
              discovery = EthDpmDeviceDiscoveryOff;
          }
          break;
      }
      case EthMiiPhyTypeBcm5400:
      case EthMiiPhyTypeBcm5208:
      case EthMiiPhyTypeLxt9782:
      case EthMiiPhyTypeMrvl88E1000:
      case EthMiiPhyTypeMrvl88E1040:
      default: {
          assert( 0 );
          break;
      }
    }

    return discovery;
}

EthDpmDeviceStatus
EthMiiPhyMan::dpmDeviceStatus() {

    EthDpmDeviceStatus dpmDeviceStatus = EthDpmDeviceStatusNull;

    if( halEthMiiDevice_->hardwareWriteVersion() !=
        halEthMiiDevice_->softwareWriteVersion() ) {
        // Not yet.
        return EthDpmDeviceStatusNull;
    }

    assert( ( dpmDeviceDiscovery() == EthDpmDeviceDiscoveryOn ) &&
            ( dpmDeviceDiscoveryModeEnable() == 
              EthDpmDeviceDiscoveryModeEnableOn ) );
    if( ( dpmDeviceDiscovery() == EthDpmDeviceDiscoveryOn ) &&
        ( dpmDeviceDiscoveryModeEnable() == 
          EthDpmDeviceDiscoveryModeEnableOn ) ) {
        switch( phyType_ ) {
          case EthMiiPhyTypeBcm5218: {
              //     DPMSTAT       DPMMISMATCH         DEVICE
              //        1               0             Telecaster
              //        0               1            Non-Dpm Device
              //        0               0           Nothing connected
              //        1               1        ???should not arise????
              HalEthExtendedStatusReg ext;
              ext.rep( ( halEthMiiDevice_->extendedStatusReg()).rep());
              HalEthBcm5218DpmReg reg;
              reg.rep( ext.rep());
              if( debugLevel_ >= 3 ) {
                  LogID(( "EthMiiPhyMan: IPstat : %d, IP mismatch : %d", 
                          reg.dpmStat(), reg.mismtch()));
              }
              if( reg.dpmStat() && reg.mismtch()) {
                  // Never ever should both of these flags be set at the
                  // same time.
                  assert( 0 );
              }
              if( reg.dpmStat() && !reg.mismtch()) {
                  dpmDeviceStatus = EthDpmDeviceStatusDpmDevicePresent;
              }
              if( !reg.dpmStat() && reg.mismtch()) {
                  dpmDeviceStatus = EthDpmDeviceStatusNonDpmDevicePresent;
              }
              if( !reg.dpmStat() && !reg.mismtch()) {
                  dpmDeviceStatus = EthDpmDeviceStatusNothingDetected;
              }
              break;
          }
          case EthMiiPhyTypeBcm5208:
          case EthMiiPhyTypeBcm5400:
          case EthMiiPhyTypeLxt9782:
          case EthMiiPhyTypeMrvl88E1000:
          case EthMiiPhyTypeMrvl88E1040:
          default: {
              assert( 0 );
              break;
          }
        }
    }
    return dpmDeviceStatus;
}

bool
EthMiiPhyMan::dpmDeviceDiscoveryCapable() {

    switch( phyType_ ) {
      case EthMiiPhyTypeBcm5218: {
          return true;
          break;
      }
      case EthMiiPhyTypeBcm5208:
      case EthMiiPhyTypeBcm5400: 
      case EthMiiPhyTypeLxt9782:
      case EthMiiPhyTypeMrvl88E1000: 
      case EthMiiPhyTypeMrvl88E1040: {
          return false;
          break;
      }
      default: {
          assert( 0 );
          break;
      }
    }
    return false;
}

// This method adds the infrastructure to display EthMiiPhyMan debug info.
// I will add the actual debug info in Wickwire. - Anshul
void
EthMiiPhyMan::showDebugInfo( Stream * s ) {

    // show common stuff here
    // ToDo : Display EthMmiPhyMan state machine details

    // show Phy specific debug info here
    switch( phyType_ ) {
      case EthMiiPhyTypeBcm5218:
        showBrcm5218DebugInfo( s );
        break;
        
      case EthMiiPhyTypeBcm5208:
        showBrcm5208DebugInfo( s );
        break;
        
      case EthMiiPhyTypeBcm5400:
        showBrcm5400DebugInfo( s );
        break;

      case EthMiiPhyTypeLxt9782:
        showLxt9782DebugInfo( s );
        break;
        
      case EthMiiPhyTypeMrvl88E1000: 
        showMrvl88E1000DebugInfo( s );
        break;
        
      case EthMiiPhyTypeMrvl88E1040:
        showMrvl88E1040DebugInfo( s );
        break;

      default:
        assert( 0 );
        break;
    }
}

void
EthMiiPhyMan::showBrcm5400DebugInfo( Stream * s ) {
    if( brcm5400AutoNegErrorFixCount_ ) {
        StreamPrintf( s, "Brcm5400AutoNegErrResolvedCount     : %10u",
                      brcm5400AutoNegErrorFixCount_ );
        StreamPrintf( s, "\n" );
    }
}

void
EthMiiPhyMan::showBrcm5218DebugInfo( Stream * s ) {

}

void
EthMiiPhyMan::showBrcm5208DebugInfo( Stream * s ) {

}

void
EthMiiPhyMan::showLxt9782DebugInfo( Stream * s ) {

}

void
EthMiiPhyMan::showMrvl88E1000DebugInfo( Stream * s ) {

}

void
EthMiiPhyMan::showMrvl88E1040DebugInfo( Stream * s ) {

}


/*
 * Copyright (c) 1998-2001,2002 by Cisco Systems, Inc.
 * All rights reserved.
 */

#include <pim/PimEthAutoNegotiator.h>
#include <event/Event.h>
#include <event/EventNotify.h>
#include <event/EventQueue.h>
#include <pim/PimPhyport.h>
#include <pim/PimSwitch.h>
#include <eth/EthAutoNegotiator.h>
#include <eth/EthDtePowerController.h>
#include <gs/Time.h>
#include <gs/Stream.h>
#include <s2w/Types.h>
#include <s2w/S2wError.h>
#include <log/Log.h>
#include <pim/LogPim.h>
#include <gs/ErrorReport.h>
#include <gs/ErrorTypes.h>

const MilliWatts DefaultCiscoInlinePowerPerPort = 6000;
const u_int      MaxOutOfControlCountsPer30Seconds = 5;
const Microseconds Time30Seconds = 30 * 1000 * 1000;

PimEthAutoNegotiator::PimEthAutoNegotiator() {
    pimPhyport_ = 0;
    ethAutoNegotiator_ = 0;

    ethDtePowerController_ = 0;
    powerOnTime_ = MicrosecondsNever;
    timeDiscoveryTurnedOnAfterDeny_ = MicrosecondsNever;
    timeFirstDtePowerOutOfControl_ = MicrosecondsNever;
    notifiedManagerOfDpmStatus_ = false;
    notifiedManagerOfPowerOnAndPowerStatusNotOk_ = false;
    hardwareErrorMessageLogged_ = false;
    clock_ = DefaultClock();
    debugLevel_ = 1;
    firstLinkDownTime_ = MicrosecondsNever;
    dpmWaitLinkupTimer_ = MicrosecondsNever;
    dpmDelayCheckTimer_ = MicrosecondsNever;
    dpmState_ = PimDpmStateNull;
    dpmDiscoveryModeConfigured_ = false;
    dtePowerOutOfControlCounts_ = 0;
    dtePowerOutOfControlSampleCounts_ = 0;
}

void
PimEthAutoNegotiator::runReview() {

    PimPhyport * pp = pimPhyport_;    

    if( pp->dpmSupported() && pp->dpmDeviceDiscoverySupported() ) {
        assert( ethDtePowerController_ );
        dpmSM->run();
    } else {
        nonDpmSM->run();
    }
}

void
dpmSM::run(){
    funtionForAllState();
    currentState_->functionBeforeRun();
    SmEvent = nextPendingEvent();
    if( SmEvent ) {
        Transition * transition = currentState_->transition( SmEvent );
        currentState_->functionAfterRun();
        transition->function();
        currentState_ = transition->nextState_; 
    } 
    lastState_->functionAfterRun();
}

void
PimEthAutoNegotiator::functionForAllState() {
    
    PimPhyport * pp = pimPhyport_;
    EthDtePowerController * dte = ethDtePowerController_;

    if( !pp->enabled() || pp->dpmAdminState() == PimDpmAdminStateOff ) {
        
        if( dpmDiscoveryModeConfigured_ ) {
            dpmDeviceDiscoveryMode( pp, false );
            dpmDiscoveryModeConfigured_ = false;
        }

        if( dpmState_ != PimDpmStateOff ) {
            dpmSM->throwEvent( SmEvent_Reset );
            // transtion refer dpmResetState();
        }
       
        assert( pp->dpmPdClass() == PimDpmPdClassNull );
        assert( pp->dpmPdPower() == 0 );
        assert( !pp->dpmPdPowerDenied() );

        if( oldState != dpmState_ ) {
            Log(( LogPimInlinePowerSmTrans,
                  pp->name().string(), oldState, dpmState_ ));
        }
        
        nonDpmSM->run();
        // runNonDpmReview();
        return;
    }
}

State PimDpmStateOff() {}
State PimDpmStateNull() {} functionBeforeRun() {
    // This can be triggered by event SmEventOff
    
    // Since admin is on now, we need to start hw detection, this may
    // bring the link down. In this state, we sync. the auto-neg state, 
    // this should not affect the detection mode and vice vesa.
    nonDpmSM->run();
    //runNonDpmReview();
    
    assert( pp->dpmPdClass() == PimDpmPdClassNull );
    assert( pp->dpmPdPower() == 0 );
    assert( !pp->dpmPdPowerDenied() );
    
    if( !dpmDiscoveryModeConfigured_ ) {
        dpmDeviceDiscoveryMode( pp, true );
        dpmDiscoveryModeConfigured_ = true;
    } else {
        // Wait until hw is in discovery enabled mode.
        if( dpmDeviceDiscoveryModeEnable() == 
            EthDpmDeviceDiscoveryModeEnableOn ) {
            
            ErrorCode linkWatchErr = Ok;
            if( !dte->currentDetectionSupported() ) {
                dte->linkDownAction( 
                    EthDteLinkDownActionDisablePower );
            } else {
                dte->currentDetectionEnabled( true );
            }
            linkWatchErr = ThisErrorReport().errorCode();
            
            if ( linkWatchErr != Ok ) {
                // set some value in EthMiiPhyMan
                dpmResetState();
                Log(( LogPimS2wErrorReport, pp->name().string() ));
                return;                     
            }
            // this is the transition.
            dpmSM->addEvent( SM_Disvoer_Enable );
            // will do
              // restartCiscoDiscovery( pp );
            // dpmState_ = PimDpmStateDetecting;            
        }
    }
}

State PimDpmStateDetecting() functionBeforeRun() {
    assert( pp->dpmPdClass() == PimDpmPdClassNull ||
            pp->dpmPdClass() == PimDpmPdClassCisco );
    assert( pp->dpmPdPower() == 0 );
    assert( dpmDiscoveryModeConfigured_ );
    
    if( eth->dpmDeviceDiscoveryModeEnable() == 
        EthDpmDeviceDiscoveryModeEnableOff ) {
        // This should not happen, since we should be the only one that
        // control the mode. But if someone changed the mode when 
        // debugging. We reset the state.
        assert( 0 );
        dpmSM->throwEvent( SmEvent_Reset );
        // dpmResetState();
        return;
    }
    
    bool newResults = false;
    
    runCiscoDiscovery( pp, &newResults );
    
    if( !newResults ) {
        dpmSM->throwEvent( SmEvent_detect_detect );
        return;
    }
    
    switch( pp->dpmPdClass() ) {
      case PimDpmPdClassNull:
        pp->dpmPdPowerDenied( false );
        break;
      case PimDpmPdClassUnknown: {
          pp->dpmPdPowerDenied( false );
          eth->restartAutoNegotiationOnConfigChange( false );
          eth->needToWaitForLinkDownOnConfigChange( false );
          assert( eth->dpmDeviceDiscovery() == 
                  EthDpmDeviceDiscoveryOn );
          eth->forcedDpmDeviceDiscovery( EthDpmDeviceDiscoveryOff, 
                                         false );
          applyUsersConfig();
          dpmLinkupTimerStart();
          dpmSM->throwEvent( SmEvent_WaitForLinkup );
          // dpmState_ = PimDpmStateWaitForLinkup;
          break;
      }
      case PimDpmPdClassCisco: {
          MilliWatts power = pp->newDpmPdPower( 
              DefaultCiscoInlinePowerPerPort );
          
          if( power != DefaultCiscoInlinePowerPerPort ) {
              assert( power == 0 );
              assert( pp->dpmPdPower() == 0 );
              assert( pp->dpmPdPowerDenied() );
              eth->restartAutoNegotiationOnConfigChange( true );
              eth->needToWaitForLinkDownOnConfigChange( true );
              restartCiscoDiscovery( pp );
              Log(( LogPimInlinePowerSmEvent, 
                    pp->name().string(), "Power Denied!" ));
          } else {
              pp->dpmPdPowerDenied( false );
              assert( pp->dpmPdPower() == power );
              
              assert( dte->dtePowerConfig() != EthDtePowerOn );
              
              dte->linkDownAction( EthDteLinkDownActionDisablePower );
              
              ErrorCode powerOffErr = Ok;
              dte->dtePowerConfig( EthDtePowerOn );
              powerOffErr = ThisErrorReport().errorCode();
              assert( dte->dtePowerConfig() == EthDtePowerOn );
              
              if( powerOffErr != Ok ) {
                  dpmResetState();
                  Log(( LogPimS2wErrorReport, pp->name().string() ));
                  return;
              }
              
              eth->forcedDpmDeviceDiscovery( EthDpmDeviceDiscoveryOff, 
                                             true );
              
              applyUsersConfig();
              
              Log(( LogPimInlinePowerSmEvent,
                    pp->name().string(), "Applied Power" ));
              
              powerOnTime_ = clock_->microTime();
              dpmLinkupTimerStart();
              SM_throwEvent( waitforLinkUp );
              // dpmState_ = PimDpmStateWaitForLinkup;
          }
          break;
      }
      default:
        assert( 0 );
        break;
    }
}
 
State PimDpmStateWaitForLinkup() {}
functionBeforeRun() {
    // check various events: time-out, power grant/deny, power applied,
    // and ultimately the link up.
    assert( eth->dpmDeviceDiscoveryModeEnable() != 
            EthDpmDeviceDiscoveryModeEnableOff );
    
    EthLinkStatus ethLinkStatus = eth->linkStatus();
    
    switch( pp->dpmPdClass() ) {
      case PimDpmPdClassCisco:
        assert( ( !pp->dpmPdPowerDenied() && pp->dpmPdPower() > 0 ) ||
                ( pp->dpmPdPowerDenied() && pp->dpmPdPower() == 0 ) );
        
        if( dte->needsReadCacheFlush()) {
            dte->flushReadCache();
        }
        
        if( pp->dpmPdPowerDenied() ) {
            // Keep power denied flag when we restart the detection.
            // The successfull detection would request power again, and
            // if we get power, we would clear the denied flag then.
            // Detection would also clear the flag if no phone device is 
            // attached.
            ErrorCode powerOffErr = Ok;
            dte->dtePowerConfig( EthDtePowerOff );
            powerOffErr = ThisErrorReport().errorCode();
            
            if( powerOffErr != Ok ) {
                // Most likely a S2W error. Since we would anyway restart
                // detection, we just fall through.
                Log(( LogPimS2wErrorReport, pp->name().string() ));
            }
            
            assert( pp->dpmPdPower() == 0 );
            eth->restartAutoNegotiationOnConfigChange( true );
            eth->needToWaitForLinkDownOnConfigChange( true );
            restartCiscoDiscovery( pp );

            SM_throwEvent( StateDetecting );
            // dpmState_ = PimDpmStateDetecting;
            Log(( LogPimInlinePowerSmEvent,
                  pp->name().string(), "Power Denied!" ));
        } else {
            
            // Wait for some time for power propagation and check link
            // status.
            if( powerOnTime_ != MicrosecondsNever &&
                ( ( clock_->microTime() - powerOnTime_ ) > 
                  MaxWaitTimeForPowerOnPropagation ) ) {
                
                applyUsersConfig();
                
                ethLinkStatus = eth->linkStatus();
                
                ErrorCode dtePowerStatusError = Ok;
                EthDtePowerOpStatus dtePowerStatus = 
                    dte->dtePowerStatus();
                dtePowerStatusError = ThisErrorReport().errorCode();
                
                if( dtePowerStatusError != Ok ) {
                    // Most likely a S2W error. Since we can't trust
                    // the read value, we just return and retry in next
                    // round.
                    Log(( LogPimS2wErrorReport, pp->name().string() ));
                    return;
                }
                if( dtePowerStatus == EthDtePowerOpStatusOutOfControl ) {
                    dtePowerOutOfControl( dpmState_ );
                    if( dpmState_ != PimDpmStateWaitForLinkup ) {
                        break;
                    }
                } else if( dtePowerStatus == EthDtePowerOpStatusOff ) {
                          // Power has been shut off by hardware. Wait until
                    // ethLink down and restart detection.
                    if( ethLinkStatus == EthLinkStatusDown ) {
                        pp->dpmPdClass( PimDpmPdClassNull );
                        if( pp->dpmPdPower() > 0 ) {
                            pp->deleteDpmPdPower( pp->dpmPdPower() );
                        }
                        assert( pp->dpmPdPower() == 0 );
                        pp->dpmPdPowerDenied( false );
                        
                        ErrorCode powerOffErr = Ok;
                        dte->dtePowerConfig( EthDtePowerOff );
                        powerOffErr = ThisErrorReport().errorCode();
                        if( powerOffErr != Ok ) {
                            dpmResetState();
                            Log(( LogPimS2wErrorReport, 
                                  pp->name().string() ));
                            return;
                        }
                        eth->restartAutoNegotiationOnConfigChange(true);
                        eth->needToWaitForLinkDownOnConfigChange(true);
                        restartCiscoDiscovery( pp );
                        dpmState_ = PimDpmStateDetecting;
                        Log(( LogPimInlinePowerSmEvent, 
                              pp->name().string(), 
                              "PowerDown when wait Linkup!" ));
                        break;
                    }
                } else if( dtePowerStatus == EthDtePowerOpStatusFaulty ){
                    // This can be valid in this state. We configured 
                    // hareware to apply power, but hardware haven't 
                    // applied power yet.
                    break;
                }
                
                if( dtePowerStatus == EthDtePowerOpStatusOn && 
                    ethLinkStatus == EthLinkStatusUp ) {
                    Log(( LogPimInlinePowerSmEvent, 
                          pp->name().string(), "LinkUP for Cisco-Phone!"));
                    SM_throwEvent( StateLinkup );
                    // dpmState_ = PimDpmStateLinkup;
                }
            } else {
                dte->needsReadCacheFlush( true );
            }
        }
        break;
      case PimDpmPdClassUnknown: {
          assert( pp->dpmPdPower() == 0 );
          updateResultsFromAutoNegotiation();
          if( ethLinkStatus == EthLinkStatusUp ) {
              Log(( LogPimInlinePowerSmEvent,
                    pp->name().string(), "LinkUP for Unknown Device!" ));
              
              eth->restartAutoNegotiationOnConfigChange( true );
              eth->needToWaitForLinkDownOnConfigChange( true );
              SM_throwEvent( StateLinkup );
              // dpmState_ = PimDpmStateLinkup;
          }
          break;
      }
      case PimDpmPdClassNull:
      default:
        assert( 0 );
    }
    
    if( ( dpmState_ == PimDpmStateWaitForLinkup ) &&
        dpmLinkupTimerTimeOut() ) {
        Log(( LogPimInlinePowerSmEvent,
              pp->name().string(), "WaitForLinkup Timed Out!" ));
        
        ErrorCode powerOffErr = Ok;
        dte->dtePowerConfig( EthDtePowerOff );
        powerOffErr = ThisErrorReport().errorCode();
        
        if( powerOffErr != Ok ) {
            dpmResetState();
            Log(( LogPimS2wErrorReport, pp->name().string() ));
            return;
        }
        
        pp->dpmPdClass( PimDpmPdClassNull );
        if( pp->dpmPdPower() > 0 ) {
            pp->deleteDpmPdPower( pp->dpmPdPower() );
        }
        
        assert( pp->dpmPdPower() == 0 );
        pp->dpmPdPowerDenied( false );
        eth->restartAutoNegotiationOnConfigChange( true );
        eth->needToWaitForLinkDownOnConfigChange( true );
        restartCiscoDiscovery( pp );
        SM_throwEvent( SM_Event_detect );
        // dpmState_ = PimDpmStateDetecting;
        break;
    }
    
    break;
}

State PimDpmStateLinup() {}
functionBeforeRun() {
    // Monitor linkdown and power down. Remove power if linkdown or
    // power down happens.
    assert( eth->dpmDeviceDiscoveryModeEnable() != 
            EthDpmDeviceDiscoveryModeEnableOff );
    
    runNonDpmReview();
    
    switch( pp->dpmPdClass() ) {
      case PimDpmPdClassCisco: {
          if( dte->dtePowerConfig() == EthDtePowerOn &&
              ( pp->dpmPdPower() == 0 ) ) {
              // We applied power, but upper layer may denied the power
              // due to power shortage or adjust the power from cdp. 
              // We need to shut down the power, we won't go directly 
              // to detection mode, instead the link down event would
              // make it restart the detection.
              Log(( LogPimInlinePowerSmEvent,
                    pp->name().string(), "Power Turned-Off!" ));
              ErrorCode powerOffErr = Ok;
              dte->dtePowerConfig( EthDtePowerOff );
              powerOffErr = ThisErrorReport().errorCode();
              if( powerOffErr != Ok ) {
                  dpmResetState();
                  Log(( LogPimS2wErrorReport, pp->name().string() ));
                  return;
              }
          }
          ErrorCode dtePowerStatusError = Ok;
          EthDtePowerOpStatus dtePowerStatus = dte->dtePowerStatus();
          dtePowerStatusError = ThisErrorReport().errorCode();
          
          EthLinkStatus ethLinkStatus = eth->linkStatus();

          ErrorCode linkStatusError = Ok;
          EthDteLinkStatus dteLinkStatus = dte->linkStatus();
          linkStatusError = ThisErrorReport().errorCode();
          
          if( ( dtePowerStatus == EthDtePowerOpStatusOutOfControl ) &&
              ( dtePowerStatusError == Ok ) ) {
              dtePowerOutOfControl( dpmState_ );
              if( dpmState_ != PimDpmStateLinkup ) {
                  break;
              }
          } else if( ( dtePowerStatusError != Ok ) || 
                     ( linkStatusError != Ok ) ) {
              // Most likely a S2W error.
              Log(( LogPimS2wErrorReport, pp->name().string() ));
          }
          
          if( ethLinkStatus == EthLinkStatusDown ) {
              pp->dpmPdClass( PimDpmPdClassNull );
              if( pp->dpmPdPower() > 0 ) {
                  pp->deleteDpmPdPower( pp->dpmPdPower() );
              }
              assert( pp->dpmPdPower() == 0 );
              pp->dpmPdPowerDenied( false );
              
              ErrorCode powerOffErr = Ok;
              dte->dtePowerConfig( EthDtePowerOff );
              powerOffErr = ThisErrorReport().errorCode();
              if( powerOffErr != Ok ) {
                  dpmResetState();
                  Log(( LogPimS2wErrorReport, pp->name().string() ));
                  return;
              }
              
              eth->restartAutoNegotiationOnConfigChange( true );
              eth->needToWaitForLinkDownOnConfigChange( true );
              restartCiscoDiscovery( pp );
              dpmState_ = PimDpmStateDetecting;
              Log(( LogPimInlinePowerSmEvent,
                    pp->name().string(), "LinkDown for IP Phone!" ));
          } else {
              if( dteLinkStatus == EthDteLinkStatusDown ||
                  dtePowerStatus == EthDtePowerOpStatusOff ) {
                  // We wait until ethLink down. and then report to pim.
                  Log(( LogPimInlinePowerSmEvent, pp->name().string(), 
                        "DteLinkDown for Phone Device!" ));
                  
              }
                }
          break;
      }
      case PimDpmPdClassUnknown: {
          if( eth->linkStatus() != EthLinkStatusUp ) {
              assert( pp->dpmPdPower() == 0 );
              pp->dpmPdClass( PimDpmPdClassNull );
              eth->restartAutoNegotiationOnConfigChange( true );
              eth->needToWaitForLinkDownOnConfigChange( true );
              restartCiscoDiscovery( pp );
              Log(( LogPimInlinePowerSmEvent, pp->name().string(),
                          "LinkDown for Unknown Device!" ));
              dpmState_ = PimDpmStateDetecting;
          }
          break;
      }
      case PimDpmPdClassNull:
      default:
        assert( 0 );
        break;
    }
    break;
}
              
void 
PimEthAutoNegotiator::runNonDpmReview() {

    PimPhyport * pp = pimPhyport_;
    EthAutoNegotiator * neg = ethAutoNegotiator_;

    EthConfigurationMode cmode;
    EthFlowControlModeSet possibleFlowControlModeSet;
    EthLineSpeedSet possibleLineSpeedSet;
    EthDuplexSet possibleDuplexSet;
    EthClockMode clockMode;
    EthFaultIndication local;
    EthConnectorType connectorType;
    EthLoopback possibleLoopback;

    if( debugLevel_ >= 4 ) {
        LogID(( "Non-DPM review" ));
    }
    if( pp ) {
        // I am currently attached to a PimPhyport.  Reconfigure the negotiator
        // based on the pimPhyport.
        
        local = pp->localFaultIndication();
        if( local == EthFaultIndicationNone ) {
            if( !pp->enabled() ) {
                local = EthFaultIndicationDisabledByUser;
            } else if( pp->flapping() ) {
                local = EthFaultIndicationLinkFailure;
            } else {
                // port looks ok --- don't report anything.
            }
        }
        
        if( !pp->enabled() ) {
            cmode = EthConfigurationModeDisabled;
        } else if( pp->autoNegotiationEnabled() ) {
            cmode = EthConfigurationModeAuto;
        } else {
            // If the port does not support forced negotiation,
            // force the configuration mode to Auto.
            // This check is needed as newer gbics like 1000BaseT
            // Texas2 gbics do not support forced negotiation and
            // it is possible to set the negotiation on a not-connected
            // forced and then insert a Texas2 gbic into that port.
            if( pp->forcedNegotiationSupported() ) {
                cmode = EthConfigurationModeForced;
            } else {
                cmode = EthConfigurationModeAuto;
            }
        }

        possibleFlowControlModeSet = pp->supportedFlowControlMode();
        possibleLineSpeedSet = pp->supportedLineSpeed();
        possibleDuplexSet = pp->supportedDuplex();
        possibleFlowControlModeSet &= pp->permittedFlowControlMode();
        possibleLineSpeedSet &= pp->permittedLineSpeed();
        possibleDuplexSet &= pp->permittedDuplex();

        clockMode = pp->permittedClockMode();
        possibleLoopback = EthLoopbackOff;
        if( pp->permittedLoopback() == EthLoopbackOn ) {
            possibleLoopback = EthLoopbackOn;
        }

        connectorType = ethConnectorType( pp->connectorType() );
        
    } else {
        // I am not currently attached to a PimPhyport.  User should not call
        // my "sync" function in this mode.  

        assert( 0 );
        return;
    }
    
    if( neg->permittedFlowControlMode() != possibleFlowControlModeSet ||
        neg->permittedLineSpeed() != possibleLineSpeedSet ||
        neg->permittedDuplex() != possibleDuplexSet ||
        neg->permittedLoopback() != possibleLoopback ||
        neg->localFaultIndication() != local ||
        neg->configurationMode() != cmode ||
        neg->negotiationStatus() == EthNegotiationStatusNotStarted ||
        neg->permittedClockMode() != clockMode ||
        neg->connectorType() != connectorType ) {
        // Negotiator needs to be reconfigured.
        RunMode oldRm = ethAutoNegotiator_->runMode();
        if( oldRm == RunModeOperating ) {
            neg->runMode( RunModeFrozen );
        }
        neg->permittedLineSpeed( possibleLineSpeedSet );
        neg->permittedDuplex( possibleDuplexSet );
        neg->permittedLoopback( possibleLoopback );
        neg->permittedFlowControlMode( possibleFlowControlModeSet );
        if( clockMode != EthClockModeNull ) {
            neg->permittedClockMode( clockMode );
        }
        neg->localFaultIndication( local );
        neg->configurationMode( cmode );
        neg->connectorType( connectorType );
        neg->runMode( oldRm );
    }
    updateResultsFromAutoNegotiation();
}

void
PimEthAutoNegotiator::updateResultsFromAutoNegotiation() {

    PimPhyport * pp = pimPhyport_;
    EthAutoNegotiator * neg = ethAutoNegotiator_;

    bool linked = (neg->linkStatus() == EthLinkStatusUp);
    EthDuplex dup;
    EthLineSpeed speed;
    EthFlowControlMode flowctrl;
    EthLoopback loop;
    EthClockMode clockMode;

    Microseconds currentTime = clock_->microTime();
    Microseconds portDebounceTime = pp->debounceTime();
    const char * portName = pp->name().string();

    if( linked ) {
        dup = neg->duplex();
        speed = neg->lineSpeed();
        loop = neg->loopback();
        flowctrl = neg->flowControlMode();
        clockMode = neg->clockMode();
        assert( dup != EthDuplexNull &&
                speed != EthLineSpeedNull &&
                loop != EthLoopbackNull &&
                flowctrl != EthFlowControlModeNull ); 
        if( portDebounceTime > 0 ) {
            // We only log from Linkdown to Linkup event
            if( !pp->linked() ) {
                Log(( LogPimLinkUp, portName, currentTime, portDebounceTime ));
            }
            firstLinkDownTime_ = MicrosecondsNever;
        }
    } else {       
        // first check debounce feature is enabled or not in this port
        if( portDebounceTime > 0 ) {            
            if ( firstLinkDownTime_ == MicrosecondsNever ) {
                // don't report the first link down
                firstLinkDownTime_ = currentTime;
                Log(( LogPimFirstLinkDownDebounce, portName, currentTime, 
                      portDebounceTime ));
                return;
            } else if( currentTime - firstLinkDownTime_  <= 
                       portDebounceTime ) {
                // Don't report since the link has been down for less than 
                // the debounce time
                Log(( LogPimLinkDownIgnoredDebounce, portName, 
                      currentTime - firstLinkDownTime_, portDebounceTime ));
                return;
            } else {
                // report link down                
                if( pp->linked() ) {
                    Log(( LogPimDownTimeGreaterDebounceTime, portName, 
                          currentTime - firstLinkDownTime_, 
                          portDebounceTime ));
                }
            }
        }
        dup = EthDuplexNull;
        speed = EthLineSpeedNull;
        loop = EthLoopbackNull;
        flowctrl = EthFlowControlModeNull;
        clockMode = EthClockModeNull;
    }
    pp->linked( linked );
    pp->duplex( dup );
    pp->loopback( loop );
    pp->lineSpeed( speed );
    pp->flowControlMode( flowctrl );
    pp->autoNegotiationError( neg->negotiationError() );
    pp->remoteFaultIndication( neg->remoteFaultIndication() );
    pp->linkDownEvents( neg->linkDownEvents() );
    pp->clockMode( clockMode );
}

EthConnectorType
PimEthAutoNegotiator::ethConnectorType( PimConnectorType pct ) {

    EthConnectorType type = EthConnectorTypeNull;

    assert( pct < PimConnectorTypeMax );
    
    switch( pct ) {

      case PimConnectorTypeNull:
        type = EthConnectorTypeNull;
        break;
      case PimConnectorTypeRj45:
        type = EthConnectorTypeRj45;
        break;
      case PimConnectorTypeRj21:
        type = EthConnectorTypeRj21;
        break;
      case PimConnectorTypeGbicEmpty:
        type = EthConnectorTypeGbicEmpty;
        break;
      case PimConnectorTypeGbicCopper:
        type = EthConnectorTypeGbicCopper;
        break;
      case PimConnectorTypeGbicTwistedPair:
        type = EthConnectorTypeGbicTwistedPair;
        break;
      case PimConnectorTypeGbicTwistedPairBroken:
        type = EthConnectorTypeGbicTwistedPairBroken;
        break;
      case PimConnectorTypeGbicShortWave:
        type = EthConnectorTypeGbicShortWave;
        break;
      case PimConnectorTypeGbicLongWave:
        type = EthConnectorTypeGbicLongWave;
        break;
      case PimConnectorTypeGbicLongHaul:
        type = EthConnectorTypeGbicLongHaul;
        break;
      case PimConnectorTypeGbicUnsupported:
        type = EthConnectorTypeGbicUnsupported;
        break;
      case PimConnectorType100MtrjShortwave:
        type = EthConnectorType100MtrjShortwave;
        break;
      case PimConnectorTypeSoftware:
        type = EthConnectorTypeSoftware;
        break;
      case PimConnectorTypeGbicExtendedReach:
        type = EthConnectorTypeGbicExtendedReach;
        break;
      case PimConnectorTypeInternal:
        type = EthConnectorTypeInternal;
        break;
      case PimConnectorTypeGbicCwdm1470:
        type = EthConnectorTypeGbicCwdm1470;
        break;
      case PimConnectorTypeGbicCwdm1490:
        type = EthConnectorTypeGbicCwdm1490;
        break;
      case PimConnectorTypeGbicCwdm1510:
        type = EthConnectorTypeGbicCwdm1510;
        break;
      case PimConnectorTypeGbicCwdm1530:
        type = EthConnectorTypeGbicCwdm1530;
        break;
      case PimConnectorTypeGbicCwdm1550:
        type = EthConnectorTypeGbicCwdm1550;
        break;
      case PimConnectorTypeGbicCwdm1570:
        type = EthConnectorTypeGbicCwdm1570;
        break;
      case PimConnectorTypeGbicCwdm1590:
        type = EthConnectorTypeGbicCwdm1590;
        break;
      case PimConnectorTypeGbicCwdm1610:
        type = EthConnectorTypeGbicCwdm1610;
        break;
      case PimConnectorTypeGbicUnapproved:
        type = EthConnectorTypeGbicUnapproved;
        break;
      case PimConnectorTypeMax:
        type = EthConnectorTypeNull;
        break;
    };
    return type;
}

void
PimEthAutoNegotiator::applyUsersConfig() {
    recordOldConfiguration();
    // Currently, we'll go according to what we might've
    // done if a non dpm discovery capable phy is present.
    runNonDpmReview();
}

void
PimEthAutoNegotiator::recordOldConfiguration() {
    PimPhyport * pp = pimPhyport_;
    oldAutoNegotiationEnable_ = pp->autoNegotiationEnabled();
    oldLineSpeedSet_ = pp->permittedLineSpeed();
    oldDuplex_ = pp->permittedDuplex();
}

bool
PimEthAutoNegotiator::configChangedSinceLastApplied() {
    PimPhyport * pp = pimPhyport_;
    
    return ( ( pp->autoNegotiationEnabled() != 
               oldAutoNegotiationEnable_ ) ||
             ( oldLineSpeedSet_ != pp->permittedLineSpeed()) ||
             ( oldDuplex_ != pp->permittedDuplex()));
}

void
PimEthAutoNegotiator::dump( Stream * s, DumpLevel dl ) {
    // non-op.
}

void
PimEthAutoNegotiator::dtePowerOutOfControl( PimDpmState state ) {
    // To avoid scare away users,  we low pass the log event.
    dtePowerOutOfControlCounts_++;
    dtePowerOutOfControlSampleCounts_++;
    if( dtePowerOutOfControlSampleCounts_ == 1 ) {
        timeFirstDtePowerOutOfControl_ = clock_->microTime();
    } else if( dtePowerOutOfControlSampleCounts_ > 
               MaxOutOfControlCountsPer30Seconds ) {
        // Get current time, if time diff less than 30sec. we declare
        // bad hardware and retry.
        Microseconds curTime = clock_->microTime();
        assert( timeFirstDtePowerOutOfControl_ != MicrosecondsNever );
        if( ( curTime - timeFirstDtePowerOutOfControl_ ) <
            Time30Seconds ) {
            dpmResetState();
            Log(( LogPimHardwareError, pimPhyport_->name().string() ));
        }
        timeFirstDtePowerOutOfControl_ = curTime;
        dtePowerOutOfControlSampleCounts_ = 1;
        
    }
    ethDtePowerController_->needsReadCacheFlush( true );
}

// Polling powered device status when hardware is ready.
// If user config ( speed/duplex, etc ) changed, restart auto-neg
// ( cisco detection use auto-neg process ).
void
PimEthAutoNegotiator::runCiscoDiscovery( PimPhyport * pp, bool * newResults ) {

    assert( dpmState_ == PimDpmStateDetecting );

    EthAutoNegotiator * eth = ethAutoNegotiator_;
    assert( eth );
    
    *newResults = false;

    if( configChangedSinceLastApplied() ) {
        restartCiscoDiscovery( pp );
        return;
    }

    if( eth->dpmDeviceDiscovery() == EthDpmDeviceDiscoveryOn ) {
        if( !pp->dpmPdPowerDenied() || dpmDelayCheckTimerTimeOut() ) {
            switch( eth->dpmDeviceStatus() ) {
              case EthDpmDeviceStatusDpmDevicePresent:
                if( pp->autoNegotiationEnabled() ) {
                    eth->forcedDpmDeviceDiscovery( EthDpmDeviceDiscoveryOff,
                                                   false );
                }

                pp->dpmPdClass( PimDpmPdClassCisco );
                *newResults = true;
                Log(( LogPimInlinePowerSmEvent,
                      pp->name().string(), "Cisco Phone Detected!" ));
                break;
              case EthDpmDeviceStatusNonDpmDevicePresent:

                pp->dpmPdClass( PimDpmPdClassUnknown );
                *newResults = true;
                Log(( LogPimInlinePowerSmEvent,
                      pp->name().string(), "Unknown Device Detected!" ));
                break;
              case EthDpmDeviceStatusNothingDetected:
                pp->dpmPdClass( PimDpmPdClassNull );
                *newResults = true;
                break;
              case EthDpmDeviceStatusNull:
                break;
              default:
                assert( 0 );
                break;
            }
        }
    } else {
        Log(( LogPimInlinePowerSmEvent,
              pp->name().string(), "Cisco detection not On yet" ));
    }
}

void 
PimEthAutoNegotiator::restartCiscoDiscovery( PimPhyport * pp ) {
    assert( eth );
    assert( pp && pp == pimPhyport_ );

    // The Pd class might be cisco if we are denied power and have
    // to restart the discovery. The discovery would update the Pd class.
    assert( pp->dpmPdClass() == PimDpmPdClassNull || 
            ( ( pp->dpmPdClass() == PimDpmPdClassCisco ) &&
              ( pp->dpmPdPowerDenied() ) ) );
    assert( pp->dpmPdPower() == 0 );

    EthLineSpeedSet lineSpeedSet;
    EthDuplexSet duplexSet;
    lineSpeedSet.empty( true );
    // If auto-neg is disabled on the port, we advertise user's speed and 
    // half duplex, this is needed since PHY needs to restart auto-neg for 
    // detection and we want link up no matter what the speed and duplex of
    // the external device is. We also record the old configuration so that
    // the configuration can be re-applied if needed when link up.
    duplexSet.empty( true );
    if( pp->autoNegotiationEnabled()) {
        lineSpeedSet = pp->supportedLineSpeed();
        duplexSet = pp->supportedDuplex();
    } else {
        lineSpeedSet = pp->permittedLineSpeed();
        duplexSet.member( EthDuplexHalf, true );
    }

    RunMode oldRm = eth->runMode();

    if( oldRm == RunModeOperating ) {
        eth->runMode( RunModeFrozen );
    }
    
    permittedLineSpeed( lineSpeedSet );
    permittedDuplex( duplexSet );
    permittedLoopback( EthLoopbackOff );
    configurationMode( EthConfigurationModeAuto );

    forcedDpmDeviceDiscovery( EthDpmDeviceDiscoveryOn, true );

    runMode( oldRm );

    recordOldConfiguration();

    dpmDelayCheckTimerStart();
}


void
PimEthAutoNegotiator::dpmDeviceDiscoveryMode( PimPhyport *pp, bool enable ) {
    assert( pp && pp == pimPhyport_ );
    EthAutoNegotiator *neg = ethAutoNegotiator_;

    if( enable ) {
        neg->forcedDpmDeviceDiscoveryModeEnable( 
            EthDpmDeviceDiscoveryModeEnableOn );
        neg->forcedDpmDeviceDiscovery( EthDpmDeviceDiscoveryOff, false );
    } else {
        neg->forcedDpmDeviceDiscovery( EthDpmDeviceDiscoveryOff, false );
        neg->forcedDpmDeviceDiscoveryModeEnable( 
            EthDpmDeviceDiscoveryModeEnableOff );
    }
}

bool
PimEthAutoNegotiator::dpmLinkupTimerTimeOut() {
    assert( dpmWaitLinkupTimer_ != MicrosecondsNever );
    assert( pimPhyport_ );
    
    Microseconds timeout = ( pimPhyport_->dpmPdPower() > 0 ) ?
        MaxTimeToLinkUpAfterPowerUp : pimPhyport_->maxTimeToLinkUp();

    return ( ( clock_->microTime() - dpmWaitLinkupTimer_ ) > timeout );
}

void
PimEthAutoNegotiator::dpmResetState() {
    PimPhyport * pp = pimPhyport_;
    EthDtePowerController * dte = ethDtePowerController_;
    assert( eth && dte && pp );

    ErrorCode powerOffErr = Ok;
    dte->dtePowerConfig( EthDtePowerOff );
    powerOffErr = ThisErrorReport().errorCode();
    
    ErrorCode linkWatchErr = Ok;
    if( !dte->currentDetectionSupported() ) {
        dte->linkDownAction( EthDteLinkDownActionNone );
    } else {
        dte->currentDetectionEnabled( false );
    }
    
    linkWatchErr = ThisErrorReport().errorCode();
    
    if( powerOffErr != Ok || linkWatchErr != Ok ) {
        Log(( LogPimS2wErrorReport, pp->name().string() ));
    }

    pp->dpmPdClass( PimDpmPdClassNull );
    if( pp->dpmPdPower() > 0 ) {
        pp->deleteDpmPdPower( pp->dpmPdPower() );
    }
    pp->dpmPdPowerDenied( false );
    dpmDeviceDiscoveryMode( pp, false );

    restartAutoNegotiationOnConfigChange( true );
    needToWaitForLinkDownOnConfigChange( true );

    dpmDiscoveryModeConfigured_ = false;
    dpmState_ = PimDpmStateOff;
}

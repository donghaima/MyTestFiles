//////////////////////////////////////////////////////////////////////////////
//  Power Management Part
// Check EDCS-195978 for protocol
/////////////////////////////////////////////////////////////////////////////

//Since we need static and auto config maximum wattage per physical
//port, we need to add one attribute and two interface in PimPhyport.h.

MilliWatts maxDpmPdPower_;
void maxDpmPdPower();
bool maxDpmPdPower( MilliWatts );

MilliWatts
PimPhyport::newDpmPdPower( MilliWatts w ) {
    assert( w > 0 );
    assert( dpmPdPower_ >= 0 );

    MilliWatts power;

    // check dpmAdminState_
    switch( dpmAdminState_ ) {
      case PimDpmAdminStateAuto:
        if( w + dpmPdPower_ > maxDpmPdPower_ ) {            
            dpmPdPowerDenied( true );
            if( dpmPdPower_ > 0 ) {
                deleteDpmPdPower( dpmPdPower_ );
                assert( dpmPdPower_ == 0 );
            }
        } else {
            // check the module local pool first.
            assert( module_ );
            power = module_->newInlinePower( w );
            // power = switch_->newInlinePower( w );
            dpmPdPower_ += power;
     
            if( power == w ) {
                dpmPdPowerDenied( false );
            } else {
                assert( power == 0 );
                dpmPdPowerDenied( true );
                if( dpmPdPower_ > 0 ) {
                    deleteDpmPdPower( dpmPdPower_ );
                    assert( dpmPdPower_ == 0 );
                }
            }
        }
        break;
      case PimDpmAdminStateStatic:
        // asked power greater than the maximum configued, so deny it.
        if( w + dpmPdPower_ > maxDpmPdPower_ ) {
            dpmPdPowerDenied( true );
            if( dpmPdPower_ > 0 ) {
                deleteDpmPdPower( dpmPdPower_ );
                assert( dpmPdPower_ == 0 );
            }
        } else {
            dpmPdPower_ += power;
            dpmPdPowerDenied( false );            
        }
        break;
      case PimDpmAdminStateOff:        
      default:
        assert( 0 );
        break;
    }       

    return dpmPdPower_;
}

MilliWatts
PimPhyport::deleteDpmPdPower( MilliWatts w ) {

    assert( dpmPdPower_ >= w );
    assert( w > 0 );
    dpmPdPower_ -= w;

    // We have to turn off power fast if requested.
    // State machine would turn on the power at proper time.
    if( w > 0 && dpmPdPower_ == 0 ) {
        PimModuleNotify * notify = module_->notify();
        if( notify ) {
            notify->onPortDtePower( this, PimDtePowerOff );
        }
        assert( ThisErrorReport().errorCode() == Ok );
    }
    
    switch( dpmAdminState_ ) {
      case PimDpmAdminStateAuto:
        // tell the global pool get the power back;
        switch_->deleteInlinePower( w );
        break;
      case PimDpmAdminStateStatic:
        break;
      case PimDpmAdminStateOff:        
      default:
        assert( 0 );
        break;
    }
    return dpmPdPower_;
}

//////////////////////////////////////////////////////////////////////////////
// For linecard local pool, we can use PimModule.h 
// add a new attribute, maxNumPowerPorts_, and two interface
// in fact, currently, we will keep all the info in switch. 
// But maybe later we will have some intellegent linecard, so 
// I try to implement the protocol now. See EDCS-195978
// VeoII Functional Specification.

u_int extraNumPowerPorts_;
u_int extraNumPowerPorts();
void extraNumPowerPorts( u_int );
void newInlinePower( MilliWatts );
void deleteInlinePower( MilliWatts );

// in PimModule.cxx
MilliWatts
PimModule::newInlinePower( MilliWatts w ) {
    // For the future usage of this function, we don't assert here.
    // Thus, no matter what maxNumPowerPorts_ we get from PimSwitch_
    // We will be safe.
    assert( extraNumPowerPorts_ == 0 );
    if( extraNumPowerPorts_ > 0 ) {
        extraNumPowerPorts_--;
    }
    MilliWatts power = switch_->newInlinePower( w );
    // if power manager want to recall the power back, they will 
    // do that in last statement. So here we need to get that power back.
    // The extraPowerNumber will always be a constant if power manager
    // doesn't recall the power back.
    if( extraNumPowerPorts_ > 0 ) {
        extraNumPowerPorts_++;
    }
}

void
PimModule::deleteInlinePower( MilliWatts w ) {
    assert( extraNumPowerPorts_ == 0 );
    return switch_->deleteInlinePower( w );
}

///////////////////////////////////////////////////////////////////////////////
// In PimSwitch.h
// We add two attributes lowerInlinePowerWaterMark_, highInlinePowerWaterMark_.
// When the available inline power is less than lowerInlinePowerWaterMark,
// global pool will recall the power from the linecard local pool.
// When the available inline power is greater than highInlinePowerWaterMark_,
// global pool will allocate the linecard local pool.

MilliWatts lowInlinePowerWaterMark_;
MilliWatts lowInlinePowerWaterMark();
void lowInlinePowerWaterMark( MilliWatts );

MilliWatts highInlinePowerWaterMark_;
MilliWatts highInlinePowerWaterMark();
void highInlinePowerWaterMark( MilliWatts );

static u_int numPowerPortsInLinecard = 0;

// Modify in PimSwitch.cxx
MilliWatts
PimSwitch::newInlinePower( MilliWatts power ) {
    assert( power > 0 );
    MilliWatts allocPower = 0;
    // Rate limiting the power allocation rate, this is due to the hardware
    // limitation that if we powering up all devices too fast, hardware can 
    // run into problems.
    Microseconds curTime = DefaultClock()->microTime();
    if( ( numPowerAppliedInCurCycle_ < MaxNumPowerAppliedInOneCycle ) ||
        ( curTime - lastPowerOnTime_ > MinTimeBetweenTwoPowerOnCycles ) ||
        ( lastPowerOnTime_ == MicrosecondsNever ) ) {

        if ( ( currentInlinePowerUsage_ + power ) < 
             totalInlinePowerAvailable_ ) {
            
            Log( ( LogPimInlinePowerMgr, "new", power ) );
            
            currentInlinePowerUsage_ += power;
            allocPower = power;
            
            if( curTime - lastPowerOnTime_ > MinTimeBetweenTwoPowerOnCycles ) {
                numPowerAppliedInCurCycle_ = 1;
            } else {
                numPowerAppliedInCurCycle_++;
            }

            lastPowerOnTime_ = curTime;

            // need to check the lower water mark
            if( totalInlinePowerAvailable_ - currentInlinePowerUsage_ < 
                lowInlinePowerWaterMark_ ) {
                // recall all the local pool in every module
                PimChassis * pimChassis = stack_->master();
                if( !pimChassis ) {
                    assert( 0 );
                    return 0;
                }                
                // In PimChassis, iterate PimSlots
                for( int i = 0; i < pimSlots_; i++ ) {
                    PimSlot * pimSlot = nthPimSlot( i );
                    if( pimSlot ) {
                        PimModule * pimModule = pimSlot->pimModule(); 
                        assert( pimModule );                        
                        pimModule.extraNumPowerPorts( 0 );
                    }
                }
            }
        }
    }
    return allocPower;
}

void
PimSwitch::deleteInlinePower( MilliWatts power ) {
    assert( power > 0 );
    assert( currentInlinePowerUsage_ >= power );
    Log( ( LogPimInlinePowerMgr, "delete", power ) );
    currentInlinePowerUsage_ -= power;

    // need to check the lower water mark
    if( totalInlinePowerAvailable_ - currentInlinePowerUsage_ > 
        highInlinePowerWaterMark_ ) {
        // recall all the local pool in every module
        PimChassis * pimChassis = stack_->master();
        if( !pimChassis ) {
            assert( 0 );
            return 0;
        }
        
        
        // In PimChassis, iterate PimSlots
        for( int i = 0; i < pimSlots_; i++ ) {
            PimSlot * pimSlot = nthPimSlot( i );
            if( pimSlot ) {
                PimModule * pimModule = pimSlot->pimModule(); 
                assert( pimModule );                        
                pimModule.extraNumPowerPorts( numPowerPortsInLinecard );
            }
        }
    }
}

// I am not sure we want to change this agrithom here or not.
// Currently seems we shut down all the ports when config 
// totalInlinePowerAvailable < currentInlinePowerUsage.
// TBD.
void
PimSwitch::totalInlinePowerAvailable( Watts power ) {
    assert( currentInlinePowerUsage_ <= totalInlinePowerAvailable_ );

    MilliWatts milliPower = power * 1000;
    if( milliPower == totalInlinePowerAvailable_ ) {
        return;
    }

    totalInlinePowerAvailable_ = milliPower;

    if( currentInlinePowerUsage_ > totalInlinePowerAvailable_ ) {
        // We need to deny power for some ports, this should be quick
        // so we don't setup a review.
        // Now, we search the ports that has inline power linearly and
        // remove power until we have enough power.
        // Alternatively, we can search ports down from PimHwPhyportIdMax.
        for( PimHwPhyportId hwPhyportId( 0 );
             ( ( hwPhyportId < PimHwPhyportsMax ) &&
               ( currentInlinePowerUsage_ > totalInlinePowerAvailable_ ) );
             hwPhyportId++ ) {

            PimPhyport * phyport = hwPhyports_[Value( hwPhyportId )];
            assert( phyport );
            MilliWatts pPower = phyport->dpmPdPower();

            if( pPower > 0 ) {
                assert( !phyport->dpmPdPowerDenied() );

                phyport->deleteDpmPdPower( pPower );
                phyport->dpmPdPowerDenied( true );
            }
        }
        
    }
    assert( currentInlinePowerUsage_ <= totalInlinePowerAvailable_ );
}


///////////////////////////////////////////////////////////////////////////
// CDP part
// Check EDCS-217671 for protocol
/////////////////////////////////////////////////////////////////////////////
/*
 * Since the CDP is not a reliable protocol, we will send these TLVs 
 * in every CDP, that means, periodically.
 * And in order to make things working correctly, we always need to make sure
 * we get PCTLV-PRTLV, in this order.
 * We will process PCTLV first, then process PRTLV. And set our PSTLV-MRTLV
 * according to PRTLV.
 * This also simplify the protocol.
 */
/*
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   |   ID                          |    Power                      |
 *   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *   0xffff id is null id, we will ignore the cdp if id == 0xffff
 */
// need to add

// ilpower_private.h
typedef struct ilpower_data {
    ilpower_admin_state admin_state;
    ilpower_device_state device_state;
    ilpower_pd_data pd_data;
    milli_watts power_supply; /* previously this is power, just change name */
    u_int16 power_supply_id;
    milli_watts power_manager_request;
    u_int16 power_manager_request_id;
} ilpower_data;

// follow the instruction in cdp.h
/*
 * - add cdp_insert_mumble() function prototype to cdp_extern.h */
extern boolean cdp_insert_power_manager_request(cdp_info_type *, idbtype *, ushort *);

boolean cdp_insert_power_manager_request (cdp_info_type *item, idbtype *idb,
                                          ushort *bytes_left)
{
    hwidbtype *hwidb;
    ilpower_hwsb * ilpsb = idb_get_hwsb(idb, HWIDB_SB_ILPOWER);
    cdp_power_type cdp_power_man_request;

    /*
     * Check if there is enough space before filling in input buffer..
     *
     */
    if (*bytes_left < CDP_TLV_OVERHEAD + CDP_POWER_MAN_REQ_LEN)
        return(FALSE);

    cdp_power_man_request.id = ilpsb->power_manager_id;
    cdp_power_man_request.power = ilpsb->power_manager_request;

    PUTSHORT(&item->code, CDP_POWER_MANAGER_REQUEST);
    PUTLONG(item->value, cdp_power_man_request);
    PUTSHORT(&item->length, CDP_TLV_OVERHEAD + CDP_POWER_MAN_REQ_LEN);
    *bytes_left -= GETSHORT(&item->length);

    return(TRUE);
}

extern boolean cdp_insert_power_supply(cdp_info_type *, idbtype *, ushort *);

/* - add cdp_insert_mumble() function body to cdp.c
 * - add cases to cdp_tlv_namestring in cdp.c
 * - add entry to add_tlv_func in cdp.c 
 */
cdp_add_func_t add_tlv_func[] = {
    NULL,
    cdp_insert_device_name,
    cdp_insert_address,
    cdp_insert_port_id,
    cdp_insert_capabilities,
    cdp_insert_version,
    cdp_insert_hardware,
    cdp_insert_dummy, 
    cdp_insert_dummy, /* cdp_insert_protocol_hello() called separately */
    cdp_insert_vtp_mgmt_domain,
    cdp_insert_native_vlan,
    cdp_insert_duplex,
    cdp_insert_dummy, /* 2-way not used */
    cdp_insert_dummy,           /* 0x0d not used */
    cdp_insert_vvid,            /* 0x0e application vlan id */
    cdp_insert_dummy,           /* 0x0f we don't send triggers */
    cdp_insert_dummy,           /* 0x10 we don't send power */
    cdp_insert_dummy,           /* 0x11 mtu not supported */
    cdp_insert_trust,           /* 0x12 extend trust */
    cdp_insert_cos,             /* 0x13 cos for untrusted ports */
    cdp_insert_dummy,           /* 0x14 sys name */
    cdp_insert_dummy,           /* 0x15 sys object */
    cdp_insert_dummy,           /* 0x16 management address */
    cdp_insert_dummy,           /* 0x17 physical location */
    cdp_insert_external_port_id,
    cdp_insert_power_supply,  /* 0x19 */
    cdp_insert_power_manager_request   /* 0x20 */
};
/*
 * - add cdp_handle_mumble() function prototype to cdp_extern.h
 * - add cdp_handle_mumble() function body to cdp2.c
 * - add cdp_handle_mumble() entry to cdp_decode_func[] in cdp2.c
 */
cdp_decode_func_t cdp_decode_func[] = {
    NULL,
    cdp_handle_device_info,
    cdp_handle_address_info,
    cdp_handle_port_id_info,
    cdp_handle_capabilities_info,
    cdp_handle_version_info,
    cdp_handle_platform_info,
    NULL, /* IP prefix */
    cdp_handle_protocol_hello,
    cdp_handle_vtp_mgmt_domain,
    cdp_handle_native_vlan,
    cdp_handle_duplex,
    NULL,  /* 2way_connectivity was removed */
    NULL,               /* 0x0d not used */
    cdp_handle_vvid,    /* 0x0e voice vlan id */
    cdp_handle_trigger, /* 0x0f trigger tlv */
    cdp_handle_power,   /* 0x10 power consumed */
    NULL,               /* 0x11 mtu not supported */
    cdp_handle_trust,   /* 0x12 extend trust */
    cdp_handle_cos,     /* 0x13 cos for untrusted ports */
    NULL,               /* 0x14 sys name */
    NULL,               /* 0x15 sys object */
    NULL,               /* 0x16 management address */
    NULL,               /* 0x17 physical location */
    cdp_handle_external_port_id_info,
    /* We add totally new function here for new inline power protocol
     * In this way, we can back compatible for old cisco phone in
     * new switch.
     */
    cdp_handle_power_consume, /* 0x19 inline power consume / PSTLV */
    cdp_handle_power_request  /* 0x20 inline power request / MRTLV */
};

/* - modify CDP_MAX_TYPE macro in cdp.h
 * - add a cdp_mumble_structure below, if needed
 * - add #define CDP_mumble_FIELD_LEN or CDP_MAX_mumble_STR_LEN below
 *
 * - add field(s) to cdp_cache_type below
 */
typedef struct cdp_power_type_ {
    u_int16 id;
    u_int16 power;
} cdp_power_type;

typedef struct cdp_cache_type_ {
    struct cdp_cache_type_ *next;   /* pointer to next entry */
    sys_timestamp expiration_time;  /* when we forget this entry */
    int entry_name_len;     /* length of entry name */
    uchar *entry_name;          /* identifies this entry */
    cdp_addr_info *addr;        /* buffer for address info */
    uchar port_id[CDP_MAX_PORT_ID_STR_LEN]; /* neighbor's outgoing interface */
    uchar port_id_len;
    uchar external_port_id[CDP_MAX_EXTERNAL_PORT_ID_STR_LEN];
    uchar external_port_id_len;
    ulong capabilities;         /* functional capabilities */
    uchar *version;         /* version information */
    uchar platform[CDP_MAX_PLATFORM_STR_LEN];  /* platform information */
    idbtype *idb;                       /* which intrface this neighber is on */
    ulong device_number;        /* device number of this device */
    cdp_protocol_hello_info *ph_list;   /* protocol hellos,
                     * cached to list on show cdp neighbor detail */
    boolean received_vtp_mgmt_domain;   /* TRUE == received vtp_mgmt TLV */
    char vtp_mgmt_domain[CDP_MAX_VTP_MGMT_DOMAIN_STR_LEN + 1 /* for NUL */];
    ushort vtp_mgmt_domain_length;
    ushort native_vlan;         /*
                     * remote port's native VLAN [0..1k/4k];
                     * 0 == not received
                     */
    ushort duplex;          /* remote port's duplex;
                     * matches MIB variable enums
                     * 1 == unknown == not received
                     * 2 == half-duplex
                     * 3 == full-duplex
                     */
    uchar   app_id;     /* appliance id for appliance vlan */
    ushort  vvid;       /* appliance vlan id */
    boolean trigger;    /* device wants to receive cdp packet */
    ulong   power;      /* power in mwatts device is using */
    boolean trust;      /* if device's other ports should be trusted */
    uchar   cos;        /* cos value for untrusted ports */
    uchar xmit_mac[IEEEBYTES];      /* mac address of the interface
                     * that transmitted the advertisement
                     */
    uchar ad_version;           /* CDP header version of the advertisement
                     * that last filled this cache entry
                     */
    cdp_power_type power_consume;
    cdp_power_type power_request;    
} cdp_cache_type;

/* - add cdp_print_mumble() prototype to cdp_extern.h
 * - add cdp_print_mumble() body to cdp_parse.c
 * - add call to cdp_print_mumble() in cdp_parse.c:show_one_cdp_neighbor(), etc.
 */
extern void cdp_print_power_consume(cdp_cache_type*);
extern void cdp_print_power_request(cdp_cache_type*);

/* - add cdp_free_mumble() function prototype to cdp_extern.h
 * - add cdp_free_mumble() function body to cdp2.c
 * - add call to cdp_free_mumble() in cdp2.c
 *???????????????????????????????????????????????? didn't see this*/
/*
 * PRTLV
 */
void cdp_handle_power_request (cdp_info_type* item,
                               cdp_cache_type* cache_entry,
                               boolean new_entry)
{
    int len;
    len = GETSHORT(&item->length);
    assert( len == CDP_TLV_OVERHEAD + 4 );

    cache_entry->power_request = GETLONG(&item->value[0]);
        
    reg_invoke_cdp_power_request_from_idb(cache_entry->idb,
                                          cache_entry->power_request.id,
                                          cache_entry->power_request.power);
}

// In GaliosIlPower.c
reg_add_cdp_power_request_from_idb(GaliosIlPower_pdPowerRequest,
                                   "inline-power pd power request");

void
GaliosIlPower_pdPowerRuquest( idbtype *idb, u_int prtlvid, ulong power ) {
    RkiosPhyport * rkiosPhyport;
    assert( idb );
    ilpower_hwsb * ilpsb = idb_get_hwsb(idb, HWIDB_SB_ILPOWER);

    rkiosPhyport = GaliosHwidb_getRkiosPhyport( idb->hwptr );
    allocatePower = RkiosPhyport_setIlpowerPdPowerRequest( rkiosPhyport, 
                                                           (MilliWatts)power );
    ilpsb->power_supply = allocatePower;
    ilpsb->power_manager_request = allocatePower;
    // since PRTLV corresponding to PSTLV
    ilpsb->power_supply_id = prtlvid;
}

// In RkiosPhyport.cxx
// return the actual allocated power
Milliwatts
RkiosPhyport::ilpowerPdPowerRequest( MilliWatts power ) {
    assert( pimPhyport_ );
    if( pimPhyport_->dpmSupported() ) {
        MilliWatts allocPower = pimPhyport_->dpmPdPower();
        assert( allocPower >= 0 );
        // Adjust the power usage if we are supplying power to 
        // the powered device.
        if( allocPower > power ) {
            return pimPhyport_->deleteDpmPdPower( allocPower - power );
        } else if( allocPower < power ) {
            return pimPhyport_->newDpmPdPower( power - allocPower );
        }        
    }
}


/*
 * cdp_handle_power
 * PCTLV
 * Add power consumption info to cache entry
 */
void cdp_handle_power_consume (cdp_info_type* item, 
                               cdp_cache_type* cache_entry,
                               boolean new_entry)
{
    int len;
    len = GETSHORT(&item->length);
    assert( len == CDP_TLV_OVERHEAD + 4 );

    cache_entry->power_consume = GETLONG(&item->value[0]);

    /* 
     * notify power manager
     */
    reg_invoke_cdp_power_consume_from_idb(cache_entry->idb,
                                          cache_entry->power_consume.id,
                                          cache_entry->power_consume.power);
}

reg_invoke_cdp_power_consume_from_idb(cache_entry->idb,
                                      pctlvid,
                                      cache_entry->power);
// In GaliosIlPower.c
reg_add_cdp_power_consume_from_idb(GaliosIlPower_pdPowerConsumed,
                                   "inline-power pd power consumed");

// In RkiosPhyport.cxx
void
RkiosPhyport::ilpowerPdPowerConsumed( MilliWatts power ) {
    assert( pimPhyport_ );
    if( pimPhyport_->dpmSupported() ) {
        MilliWatts allocPower = pimPhyport_->dpmPdPower();
        assert( allocPower >= 0 );
        if( allocPower > power ) {
            pimPhyport_->deleteDpmPdPower( allocPower - power );
        } else if( allocPower < power ) {
            // There is something wrong, the device consume more than
            // we supply, so we just shutdown the device.
            pimPhyport_->deleteDpmPdPower( power - allocPower );
        }        
    }
}


//////////////////////////////////////////////////////////////////////////
// currently we have
In ilpower.c
// this is for ilpower_pd_class change
reg_add_ilpower_pd_statechange(ilpower_pd_statechange,
                               "inline-power pd state change");

// This is for device state change, for
typedef enum ilpower_device_state {
    ilpower_device_state_attached_ok,
    ilpower_device_state_attached_faulty,
    ilpower_device_state_detached,
} ilpower_device_state;
reg_add_ilpower_device_statechange(ilpower_device_statechange,
                                   "inline-power device state change");
// this is change for milliwatts
reg_add_ilpower_powerchange(ilpower_powerchange, 
                            "inline-power power change");
// in GaliosIlPower.c
reg_add_ilpower_admin_statechange(GaliosIlPower_adminStateChange,
                                  "inline-power admin state change");
// this if for SNMP???
reg_add_ilpower_get_admin_state(ilpower_get_admin_state,
                                "inline-power get admin state");
reg_add_ilpower_set_admin_state(ilpower_set_admin_state,
                                "inline-power set admin state");
reg_add_ilpower_get_oper_state(ilpower_get_oper_state,
                               "inline-power get oper state");
reg_add_ilpower_get_power(ilpower_get_power,
                          "inline-power get power");


///////////////////////////////////////////////////////////////////////////
// currently we don't have, but maybe don't need.
// we don't have this now, this is for
typedef enum ilpower_pd_power_state {
    ilpower_pd_power_off,
    ilpower_pd_power_on,
    ilpower_pd_power_faulty,
    ilpower_pd_power_deny,
} ilpower_pd_power_state;
static void
ilpower_pd_power_statechange (hwidbtype *idb, 
                              ilpower_pd_power_state pd_power_state);

// don't have this either
// this is for
typedef enum ilpower_pd_phys_state {
    ilpower_pd_absent,
    ilpower_pd_present,
} ilpower_pd_phys_state;
reg_add_ilpower_pd_phys_statechange(ilpower_pd_phys_statechange,
                                    "ilpower_pd_phys_statechange");

// Currently following only used to change the name of pd
reg_add_cdp_receive_notify(ilpower_get_cdp_notify,
                           "inline-power get cdp notify");
reg_add_cdp_purge_notify(ilpower_cdp_purge_notify,
                         "ilpower_cdp_purge_notify");

// seems this only used by 6k, not sure it's correct or not.
reg_add_ilpower_supported(ilpower_supported,
                          "inline-power supported");

/////////////////////////////////////////////////////////////////////////////
// State Machine part
// Refer other files in this directory.
// Check EDCS-198877 for spec.
/////////////////////////////////////////////////////////////////////////////
/*
 * in pim/TypesClean.h
 */
typedef enum PimDpmPdClass {
    PimDpmPdClassNull,
    PimDpmPdClassUnknown,
    PimDpmPdClassCisco,
    PimDpmPdClassIEEE0,
    PimDpmPdClassIEEE1,
    PimDpmPdClassIEEE2,
    PimDpmPdClassIEEE3,
    PimDpmPdClassIEEE4,
    PimDpmPdClassIEEE5,
} PimDpmPdClass;

const MilliWatts DefaultIEEE0InlinePowerPerPort = 15000;
const MilliWatts DefaultIEEE1InlinePowerPerPort = 4000;
const MilliWatts DefaultIEEE2InlinePowerPerPort = 7000;
const MilliWatts DefaultIEEE3InlinePowerPerPort = 15000;
// left for future expansion of next page, but unused today.
const MilliWatts DefaultIEEE4InlinePowerPerPort = 15000;
const MilliWatts DefaultIEEE5InlinePowerPerPort = 15000;

void
PimEthAutoNegotiator::runDpmReview() {

    PimPhyport * pp = pimPhyport_;
    EthAutoNegotiator * eth = ethAutoNegotiator_;
    EthDtePowerController * dte = ethDtePowerController_;

    PimDpmState oldState = dpmState_;

    if( !pp->enabled() || pp->dpmAdminState() == PimDpmAdminStateOff ) {
        
        if( dpmDiscoveryModeConfigured_ ) {
            dpmDeviceDiscoveryMode( pp, false );
            dpmDiscoveryModeConfigured_ = false;
        }

        if( dpmState_ != PimDpmStateOff ) {
            dpmResetState();
        }
       
        assert( pp->dpmPdClass() == PimDpmPdClassNull );
        assert( pp->dpmPdPower() == 0 );
        assert( !pp->dpmPdPowerDenied() );

        if( oldState != dpmState_ ) {
            Log(( LogPimInlinePowerSmTrans,
                  pp->name().string(), oldState, dpmState_ ));
        }

        runNonDpmReview();
        return;
    }

    assert( pp->dpmAdminState() == PimDpmAdminStateAuto );
    
    switch( dpmState_ ) {
      case PimDpmStateNull:
      case PimDpmStateOff: {
          // Since admin is on now, we need to start hw detection, this may
          // bring the link down. In this state, we sync. the auto-neg state, 
          // this should not affect the detection mode and vice vesa.
          runNonDpmReview();

          assert( pp->dpmPdClass() == PimDpmPdClassNull );
          assert( pp->dpmPdPower() == 0 );
          assert( !pp->dpmPdPowerDenied() );

          if( !dpmDiscoveryModeConfigured_ ) {
              dpmDeviceDiscoveryMode( pp, true );
              dpmDiscoveryModeConfigured_ = true;
          } else {
              // Wait until hw is in discovery enabled mode.
              if( eth->dpmDeviceDiscoveryModeEnable() == 
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
                      dpmResetState();
                      Log(( LogPimS2wErrorReport, pp->name().string() ));
                      return;                     
                  }
                  // !!!!!!!!!!!
                  // need to add IEEE discovery here.
                  restartDiscovery( pp );
                  dpmState_ = PimDpmStateDetecting;
              }
          }
          break;
      }
      // the following will finish the work of detecting and classification
      case PimDpmStateDetecting: {
          // Need to change 
          assert( pp->dpmPdClass() == PimDpmPdClassNull ||
                  pp->dpmPdClass() == PimDpmPdClassCisco ||
                  pp->dpmPdClass() == PimDpmPdClassIEEE0 ||
                  pp->dpmPdClass() == PimDpmPdClassIEEE1 ||
                  pp->dpmPdClass() == PimDpmPdClassIEEE2 ||
                  pp->dpmPdClass() == PimDpmPdClassIEEE3 ||
                  pp->dpmPdClass() == PimDpmPdClassIEEE4 ||
                  pp->dpmPdClass() == PimDpmPdClassIEEE5 );

          assert( pp->dpmPdPower() == 0 );
          assert( dpmDiscoveryModeConfigured_ );

          if( eth->dpmDeviceDiscoveryModeEnable() == 
              EthDpmDeviceDiscoveryModeEnableOff ) {
              // This should not happen, since we should be the only one that
              // control the mode. But if someone changed the mode when 
              // debugging. We reset the state.
              assert( 0 );
              dpmResetState();
              break;
          }

          bool newResults = false;
          
          // change to runDiscovery() !!!!!
          // so need to change a lot in EthMiiPhyMan, add phyType_ there
          // everywhere with EthMiiPhyTypeBcm5218
          runDiscovery( pp, &newResults );

          if( !newResults ) {
              break;
          }
          
          MilliWatts requestPower = 0;

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
                dpmState_ = PimDpmStateWaitForLinkup;
                break;
            }
            case PimDpmPdClassCisco:
            case PimDpmPdClassIEEE0: 
            case PimDpmPdClassIEEE1: 
            case PimDpmPdClassIEEE2: 
            case PimDpmPdClassIEEE3: 
            case PimDpmPdClassIEEE4: 
            case PimDpmPdClassIEEE5: {
            
                if( pp->dpmPdClass() == PimDpmPdClassCisco ) { 
                    requestPower = DefaultCiscoInlinePowerPerPort;
                } else if( pp->dpmPdClass() == PimDpmPdClassIEEE0 ) {
                    requestPower = DefaultIEEE0InlinePowerPerPort;
                } else if ( pp->dpmPdClass() == PimDpmPdClassIEEE1 ) {
                    requestPower = DefaultIEEE1InlinePowerPerPort;
                } else if ( pp->dpmPdClass() == PimDpmPdClassIEEE2 ) {
                    requestPower = DefaultIEEE2InlinePowerPerPort;
                } else if ( pp->dpmPdClass() == PimDpmPdClassIEEE3 ) {
                    requestPower = DefaultIEEE3InlinePowerPerPort;
                } else if ( pp->dpmPdClass() == PimDpmPdClassIEEE4 ) {
                    requestPower = DefaultIEEE4InlinePowerPerPort;
                } else {
                    requestPower = DefaultIEEE5InlinePowerPerPort;
                }
                
                MilliWatts power = pp->newDpmPdPower( 
                    requestPower );
                
                if( power != requestPower ) {
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
                    
                    /// ????????????????/
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
                    dpmState_ = PimDpmStateWaitForLinkup;
                }
                break;            
            }
            default:
              assert( 0 );
              break;
          }

          break;
      }
      case PimDpmStateWaitForLinkup: {
          // check various events: time-out, power grant/deny, power applied,
          // and ultimately the link up.
          assert( eth->dpmDeviceDiscoveryModeEnable() != 
                  EthDpmDeviceDiscoveryModeEnableOff );

          EthLinkStatus ethLinkStatus = eth->linkStatus();

          switch( pp->dpmPdClass() ) {

            case PimDpmPdClassCisco:
            case PimDpmPdClassIEEE0:
            case PimDpmPdClassIEEE1: 
            case PimDpmPdClassIEEE2:
            case PimDpmPdClassIEEE3:
            case PimDpmPdClassIEEE4: 
            case PimDpmPdClassIEEE5:

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
                  // change to restartDiscovery( pp );
                  restartCiscoDiscovery( pp );
                  dpmState_ = PimDpmStateDetecting;
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
                              pp->name().string(), 
                                    "LinkUP for Cisco/IEEE Phone!"));
                              dpmState_ = PimDpmStateLinkup;
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
                  dpmState_ = PimDpmStateLinkup;
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
              dpmState_ = PimDpmStateDetecting;
              break;
          }

          break;
      }
      case PimDpmStateLinkup: {
          // Monitor linkdown and power down. Remove power if linkdown or
          // power down happens.
          assert( eth->dpmDeviceDiscoveryModeEnable() != 
                  EthDpmDeviceDiscoveryModeEnableOff );

          runNonDpmReview();
          
          switch( pp->dpmPdClass() ) {
            case PimDpmPdClassIEEE0:
            case PimDpmPdClassIEEE1: 
            case PimDpmPdClassIEEE2:
            case PimDpmPdClassIEEE3:
            case PimDpmPdClassIEEE4: 
            case PimDpmPdClassIEEE5:
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
      default:
        assert( 0 );
        break;
    }
    if( oldState != dpmState_ ) {
        Log(( LogPimInlinePowerSmTrans,
              pp->name().string(), oldState, dpmState_ ));
    }
}


////////////////////////////////////////////////////////////////////////////
// For hardware support files
// Please check with Kranthi Bathula <kbathula@cisco.com>'s doc.
////////////////////////////////////////////////////////////////////////////


// to do list ??????????????????
// in GaliosSnmpEntity.c   getSnmpOid()
case C4KChassisWsx4248Rj45V:
// return getSnmpOid_Oid( IDcevCat4kWsx4248Rj45V, 13 );

// in GalLtcMan.cxx
virtual EthDteLinkDownAction linkDownAction() {
    assert( runMode_ == RunModeOperating );
    return EthDteLinkDownActionDisablePower;
    //return me_->linkDownAction( id_ );
}

virtual void linkDownAction( EthDteLinkDownAction action ) {
    assert( runMode_ == RunModeOperating );
    //me_->linkDownAction( id_, action );
}

// Polling powered device status when hardware is ready.
// If user config ( speed/duplex, etc ) changed, restart auto-neg
// ( cisco detection use auto-neg process ).
void
PimEthAutoNegotiator::runDiscovery( PimPhyport * pp, bool * newResults ) {

    assert( dpmState_ == PimDpmStateDetecting );

    EthAutoNegotiator * eth = ethAutoNegotiator_;
    EthDtePowerController * dte = ethDtePowerController_;

    assert( eth );
    
    *newResults = false;

    if( configChangedSinceLastApplied() ) {
        restartDiscovery( pp );
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
                // change this to handle both cisco phone and noncisco phone
                EthDpmPortClass ethDpmClass = dte->dpmPortClass();
                pp->dpmPdClass( ethDpmClass );
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
PimEthAutoNegotiator::restartDiscovery( PimPhyport * pp ) {
    EthAutoNegotiator * eth = ethAutoNegotiator_;
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
    
    eth->permittedLineSpeed( lineSpeedSet );
    eth->permittedDuplex( duplexSet );
    eth->permittedLoopback( EthLoopbackOff );
    eth->configurationMode( EthConfigurationModeAuto );

    eth->forcedDpmDeviceDiscovery( EthDpmDeviceDiscoveryOn, true );

    eth->runMode( oldRm );

    recordOldConfiguration();

    dpmDelayCheckTimerStart();
}

// src-galaxy/lib/eth/Type.h
enum EthDpmPortClass {
    EthDpmPortClassUnknown = 0,
    EthDpmPortClass1 = 1,
    EthDpmPortClass2 = 2,
    EthDpmPortClass3 = 3,
    EthDpmPortClass4 = 4,
    EthDpmPortClassUndefined = 5,
    EthDpmPortClass0 = 6,
    EthDpmPortClassOvercurrent = 7,
};

// in EthDtePowerController.h
virtual EthDpmPortClass dpmPortClass() = 0;

/////////////////////////////////////////////////////////////////////////////


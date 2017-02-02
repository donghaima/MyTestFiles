#!/bin/bash

# Log msg to screen and to log file
function echo_log {
    echo "$1" | tee -a $CME_INSTALL_LOG
}  

# Log msg to screen and to log file, and exit
function echo_log_exit {
    echo_log
    echo_log "$1" 
    echo_log "Installation terminated at `date`"
    [[ ! -z $TMPDIR ]] && rm -rf $TMPDIR
    exit 1
}  

CME_INSTALL_LOG=/tmp/cme-install.log
# Only log msg to the log file
function log_only {
    echo "$1" >> $CME_INSTALL_LOG
}  

# Prompt user for y/n answer, exit on n, continue on y
function prompt_user {
   if [[ "$force" == "0" ]] ; then
     # No force so prompt user for decision 
     while [ 1 ]
     do
        echo
        echo -n "$1 y/n: " | tee -a $CME_INSTALL_LOG
        read answer
        if [ "$answer" != "y" -a "$answer" != "n" ] ; then
           echo_log "Invalid answer detected. Please answer y or n only."
        else 
           break
        fi
     done
   else
     # Force continue
     answer="y"
   fi
   # Exit script for n answer, otherwise continue
   if [ "$answer" == "n" ]; then
      echo_log_exit "User terminated installation"
   else
      log_only ""
      log_only "User continuing installation"
   fi
}


# Prompt user for y/n answer. Pass in a second argument "no_exit" if the caller
# only wants to get the $answer back.
function prompt_user {
   if [[ "$force" == "0" ]] ; then
     # No force so prompt user for decision 
     while [ 1 ]
     do
        echo
        echo -n "$1 y/n: " | tee -a $CME_INSTALL_LOG
        read answer
        if [ "$answer" != "y" -a "$answer" != "n" ] ; then
           echo_log "Invalid answer detected. Please answer y or n only."
        else 
           break
        fi
     done
   else
     # Force continue
     answer="y"
   fi

   # 
   if [ "$2" == "no_exit" ]; then
      log_only ""
      log_only "User answered ${answer} to question \"$1\""
      return
   fi

   # Exit script for n answer, otherwise continue
   if [ "$answer" == "n" ]; then
      echo_log_exit "User terminated installation"
   else
      log_only ""
      log_only "User continuing installation"
   fi
}



# Apache Tomcat keystore 
TOMCAT_KEYSTORE_FILE=/tmp/packager-cert
KEYSTORE_PASSWD=encAdm1n
KEYSTORE_VALIDITY=3650  # ten years
function install_tomcat_keystore
{
   #set -x

   if [[ -f "$TOMCAT_KEYSTORE_FILE" ]]; then
      echo_log "Tomcat keystore file \"${TOMCAT_KEYSTORE_FILE}\" exists"
      prompt_user "Do you wish to continue to use the existing \"${TOMCAT_KEYSTORE_FILE}\"?" "no_exit"
      echo_log "Ask later if user wants to re-generate a keystore file - need to remove existing file first!"

   else
      # Generate one
      echo_log "Generate a keystore file ${TOMCAT_KEYSTORE_FILE} with blank dname..."
      ${KEYTOOL} -genkey -noprompt -alias captomcat \
          -dname "CN=, OU=, O=, L=, S=, C="  \
          -keystore ${TOMCAT_KEYSTORE_FILE} \
          -storepass ${KEYSTORE_PASSWD} -keypass ${KEYSTORE_PASSWD} \
          -validity ${KEYSTORE_VALIDITY}

      echo_log "Ask later if a user wants to generate the keystore file with a blank or a custom dname"
   fi

    #set +x
}


force=0

# Ensure required external tools exist on the system
KEYTOOL=/usr/bin/keytool
EXTERNAL_TOOLS=(\
   "${KEYTOOL}"\
    "/usr/cisco/bin/notkeytool"\
    "/usr/cisco/bin/keytool"\
)
# If any of the required external tools is missing, abort the install
echo 
echo_log "Performing external tool checks..."
arrlen=${#EXTERNAL_TOOLS[@]}
for (( i=0; i<$arrlen; i=i+1 ));
do
   tool=${EXTERNAL_TOOLS[$i]}
    if [[ -x "${tool}" ]]; then
        echo_log " Found \"${tool}\""
    else
        echo_log " \"${tool}\" is not found. "
        fail=1
        break
    fi
done

echo_log "Install tomcat keystore file"
install_tomcat_keystore

echo_log ""
echo_log "The end"


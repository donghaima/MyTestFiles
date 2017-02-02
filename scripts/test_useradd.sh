#!/bin/sh

groupadd -fr -g 499 vqes 1>/dev/null 2>&1

# If useradd returns anything other than 0 (success) and 9 (user already exists)
# exit with error code 1
useradd -g 499 -u 499 -c "VQES daemons" -d /etc/opt/vqes -s /sbin/nologin vqes 1>/dev/null 2>&1
echo "return code: $?"

if [ $? -ne 0 -a $? -ne 9 ]; then
    echo "useradd vqes failed!"
    exit 1
fi

useradd -g 499 -u 498 -c "VQES operator" vqe 1>/dev/null 2>&1
echo "return code: $?"
if [ "$?" -ne 0 -a "$?" -ne 9 ]; then
    echo "useradd vqe failed!"
    exit 1
fi

exit 0



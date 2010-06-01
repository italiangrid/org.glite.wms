#!/bin/sh

#
# bug #52146
#
# The ExistException is not handled properly by the client
#

if [ -z "$1" ]; then
	host=`hostname`
else
	host=$1
fi

ENDPOINT=https://$host:8443/glite-data-transfer-fts/services/ChannelManagement

echo "Using endpoint $ENDPOINT"
echo

# First attemp
OUTPUT=`glite-transfer-group-addmember -s $ENDPOINT A_GROUP A_MEMBER 2>&1`
if [ $? -ne 0 ]; then
	echo "The user and group could not be created:"
	echo $OUTPUT
	exit 1
fi


# Second attemp will force the error
OUTPUT=`glite-transfer-group-addmember -s $ENDPOINT A_GROUP A_MEMBER 2>&1`
if [ $? -eq 0 ]; then
	echo "The second attemp should have failed"
	echo $OUTPUT
	exit 1
fi

# We have an error. Check if it is the right message
if [ "x$OUTPUT" == "xFailed adding site 'A_MEMBER' to members of group '[A_GROUP]': addGroupMember: This groupMember already exists!" ]; then
	echo "Test successfull"
	retval=0
else
	echo "The exception is not handled properly:"
	echo $OUTPUT
	retval=1
fi

# Remove
glite-transfer-group-removemember -s $ENDPOINT A_GROUP A_MEMBER
if [ $? -ne 0 ]; then
	echo "Error when removing the member"
else
	echo "User removed"
fi

exit $retval

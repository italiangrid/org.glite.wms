#!/bin/sh

# Test lcg-ManageVOTag

function myexit() {

  if [ $1 -ne 0 ]; then
    echo " *** something went wrong *** "
    echo " *** test NOT passed *** "
    exit $1
  else
    echo ""
    echo "    === test PASSED === "
  fi
   
  exit 0
}

function myecho()
{
  echo "#lcg-ManageVOTag test# $1"
}

usage() {
 echo
 echo "Test to test the lcg-ManageVOTag command"
 echo "Usage:"
 echo "======"
 echo "UI-tag-managevotag.sh --ce <CEname> --vo <VO> [--extended]"
 echo ""
 echo "The --extended flag should be used if the tester has"
 echo "privileges to modify the tags of the ce"
 echo ""
}

haveCE=0
haveVO=0
extended=0

while [ $# -gt 0 ]
do
 case $1 in
 --ce | -ce ) ce=$2
  haveCE=1
  shift 
  ;;
 --vo | -vo ) vo=$2
  haveVO=1
  shift
  ;;
 --extended | -extended ) extended=$2
  ;;
 --help | -help | --h | -h ) usage
  exit 0
  ;;
 --* | -* ) echo "$0: invalid option $1" >&2
  usage
  exit 1
  ;;
 *) break
  ;;
 esac
 shift
done

if [ $haveCE -eq 0 ] || [ $haveVO -eq 0 ] ; then
 usage
 exit 1
fi

myecho "Listing available tags"

lcg-ManageVOTag -host $ce -vo $vo --list

if [ $? -ne 0 ] ; then
 myecho "The listing of vo tags failed"
 myexit 1
fi

myecho "Listing of vo tags succeeded"

myecho "Trying to list tags from a nonexisting server"

lcg-ManageVOTag -host no.such.host.cern.ch -vo $vo --list

if [ $? -eq 1 ] ; then
 myecho "The command does not return an error on using a nonexistent server"
 myexit 1
fi

myecho "The command correctly returns an error on a nonexisting server"

if [ $extended -eq 0 ] ; then
 myecho "Not running extended tests"
 myexit 0
else
 myecho "Running extended tests"
 myecho "Trying to add a tag"

 lcg-ManageVOTag -host $ce -vo $vo --add -tag VO-$vo-certtest

 if [ $? -ne 0 ]; then
  myecho "ERROR: Could not add tag VO-$vo-certtest"
  myexit 1
 fi

 myecho "Succesfully added tag VO-$vo-certtest"

 myecho "Trying to add tag from file"

 mytmp=`mktemp`
 
 if [$? -ne 0 ] ; then
  myecho "Error creating required temporary file, trying to remove added tag"
  lcg-ManageVOTag -host $ce -vo $vo --remove -tag VO-$vo-certtest
  myexit 1
 fi

 echo "VO-$vo-certtest2" >> $mytmp
 
 lcg-ManageVOTag -host $ce -vo $vo --add -file  $mytmp

 if [ $? -ne 0 ]; then
  myecho "ERROR: Could not add tag VO-$vo-certtest2 from file. Trying to remove added tag"
  lcg-ManageVOTag -host $ce -vo $vo --remove -tag VO-$vo-certtest
  rm $mytmp
  myexit 1
 fi
 
 myecho "Succesfully added tag VO-$vo-certtest2 tag from file"

 myecho "Trying to rename a tag"

 lcg-ManageVOTag -host $ce -vo $vo --replace -tag VO-$vo-certtest VO-$vo-certtest3
 if [ $? -ne 0 ]; then
  myecho "ERROR: Could not rename tag VO-$vo-certtest to VO-$vo-certtest3. Trying to remove added tags"
  lcg-ManageVOTag -host $ce -vo $vo --remove -tag VO-$vo-certtest
  lcg-ManageVOTag -host $ce -vo $vo --remove -tag VO-$vo-certtest2
  rm $mytmp
  myexit 1
 fi

 myecho "Trying to remove tag"
 lcg-ManageVOTag -host $ce -vo $vo --remove -tag VO-$vo-certtest3

 if [ $? -ne 0 ]; then
  myecho "ERROR: Could not remove tag VO-$vo-certtest3. Trying to remove other added tags"
  lcg-ManageVOTag -host $ce -vo $vo --remove -tag VO-$vo-certtest2
  rm $mytmp
  myexit 1
 fi

 myecho "Succesfully removed tag VO-$vo-certtest3"
 myecho "Trying to remove tag from file"

 lcg-ManageVOTag -host $ce -vo $vo --remove -file $mytmp

 if [ $? -ne 0 ]; then
  myecho "ERROR: Could not remove tag VO-$vo-certtest2 from file. Trying to remove from command line"
  lcg-ManageVOTag -host $ce -vo $vo --remove -tag VO-$vo-certtest2
  rm $mytmp
  myexit 1
 fi
 myecho "Succesfully removed tag from file"

 rm $mytmp

 myecho "Trying to add a badly formatted tag"
 
 lcg-ManageVOTag -host $ce -vo $vo --add -tag "VO-tagname-!!"

 if [ $? -eq 0 ]; then
  myecho "ERROR: The tagname VO-tagname-!! was accepted, trying to remove it"
  lcg-ManageVOTag -host $ce -vo $vo --remove -tag VO-tagname-!!
  myexit 1
 fi
 myecho "Correctly received and error on a badly formatted tag"

 myecho "Trying to remove a nonexistent tag"
 
 lcg-ManageVOTag -host $ce -vo $vo --remove -tag "VO-$vo-imprettysurethatthistagdoesntexist"

 if [ $? -eq 0 ]; then
  myecho "ERROR: Did not receive an error when trying to remove VO-$vo-imprettysurethatthistagdoesntexist"
  myexit 1
 fi
 myecho "Correctly received and error when trying to remove VO-$vo-imprettysurethatthistagdoesntexist"

 myexit 0

fi

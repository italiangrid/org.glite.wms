#!/bin/sh

# Test lcg-tags

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
  echo "#lcg-tags test# $1"
}

usage() {
 echo
 echo "Test to test the lcg-tags command"
 echo "Usage:"
 echo "======"
 echo "UI-tag-lcgtags.sh --ce <CEname> --vo <VO> [--sc <Subcluster name>][--extended]"
 echo ""
 echo "If the --sc flag is given, basic tests are done also by giving the" 
 echo "subcluster instead of the CE to the command"
 echo "The --extended flag should be used if the tester has"
 echo "privileges to modify the tags of the ce"
 echo ""
}

haveCE=0
haveVO=0
haveSC=0
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
 --sc | -sc ) sc=$2
  haveSC=1
  shift
  ;;
 --extended | -extended ) extended=1
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

lcg-tags --ce $ce --vo $vo --list

if [ $? -ne 0 ] ; then
 myecho "The listing of vo tags failed"
 myexit 1
fi

myecho "Listing of vo tags succeeded"

myecho "Trying to list tags from a nonexisting server"

lcg-tags --ce no.such.host.cern.ch --vo $vo --list

if [ $? -eq 1 ] ; then
 myecho "The command does not return an error on using a nonexistent server"
 myexit 1
fi

myecho "The command correctly returns an error on a nonexisting server"

if [ $haveSC -eq 1 ] ; then
  myecho "Running  list test using subcluster"
  lcg-tags --sc $sc --vo $vo --list
  if [ $? -ne 0 ] ; then 
    myecho "The list test using subcluster failed"
    myexit 1
  fi
  myecho "List test using subcluster succeeded"
fi

if [ $extended -eq 0 ] ; then
 myecho "Not running extended tests"
 myexit 0
else
 myecho "Running extended tests"
 myecho "Trying to add a tag"

 lcg-tags --ce $ce --vo $vo --add --tags VO-$vo-certtest

 if [ $? -ne 0 ]; then
  myecho "ERROR: Could not add tag VO-$vo-certtest"
  myexit 1
 fi

 myecho "Succesfully added tag VO-$vo-certtest"

 myecho "Trying to add tag from file"

 mytmp=`mktemp`
 
 if [ $? -ne 0 ] ; then
  myecho "Error creating required temporary file, trying to remove added tag"
  lcg-tags --ce $ce --vo $vo --remove --tags VO-$vo-certtest
  myexit 1
 fi

 echo "VO-$vo-certtest2" >> $mytmp
 
 lcg-tags --ce $ce --vo $vo --add --tagfile  $mytmp

 if [ $? -ne 0 ]; then
  myecho "ERROR: Could not add tag VO-$vo-certtest2 from file. Trying to remove added tag"
  lcg-tags --ce $ce --vo $vo --remove --tags VO-$vo-certtest
  rm $mytmp
  myexit 1
 fi
 
 myecho "Succesfully added tag VO-$vo-certtest2 tag from file"

 myecho "Trying to remove tag"
 lcg-tags --ce $ce --vo $vo --remove --tags VO-$vo-certtest

 if [ $? -ne 0 ]; then
  myecho "ERROR: Could not remove tag VO-$vo-certtest. Trying to remove other added tags"
  lcg-tags --ce $ce --vo $vo --remove --tags VO-$vo-certtest2
  rm $mytmp
  myexit 1
 fi

 myecho "Succesfully removed tag VO-$vo-certtest"
 myecho "Trying to remove tag from file"

 lcg-tags --ce $ce --vo $vo --remove --tagfile $mytmp

 if [ $? -ne 0 ]; then
  myecho "ERROR: Could not remove tag VO-$vo-certtest2 from file. Trying to remove from command line"
  lcg-tags --ce $ce --vo $vo --remove --tags VO-$vo-certtest2
  rm $mytmp
  myexit 1
 fi
 myecho "Succesfully removed tag from file"

 rm $mytmp

 myecho "Trying to add a badly formatted tag"
 
 lcg-tags --ce $ce --vo $vo --add --tags "VO-tagname-!!"

 if [ $? -eq 0 ]; then
  myecho "ERROR: The tagname VO-tagname-!! was accepted, trying to remove it"
  lcg-tags --ce $ce --vo $vo --remove --tags VO-tagname-!!
  myexit 1
 fi
 myecho "Correctly received and error on a badly formatted tag"

 if [ $haveSC -eq 1 ] ; then
  myecho "Running add and remove tests using subclusters"
  lcg-tags --sc $sc --vo $vo --add --tags VO-$vo-certtest
  if [ $? -ne 0 ] ; then
   myecho "Error adding tag VO-$vo-certtest to subcluster $sc"
   myexit 1
  fi
   
  myecho "Succesfully added tag VO-$vo-certtest to subcluster $sc"

  myecho "Trying to remove tag from subcluster"

  lcg-tags --sc $sc --vo $vo --remove --tags VO-$vo-certtest

  if [ $? -ne 0 ] ; then
   myecho "Error removing tag VO-$vo-certtest from subcluster $sc"
   myexit 1
  fi
  
  myecho "Succesfully removed tag VO-$vo-certtest from subcluster $sc"
 fi 


 myexit 0

fi

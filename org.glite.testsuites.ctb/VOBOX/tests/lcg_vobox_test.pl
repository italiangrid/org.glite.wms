#!/usr/bin/perl
####################################################################
# Script to test the LCG services inside the VOBOX.
# The will generate one directory named 
# $node in /tmp and in this dir. 3 files for each test will be created.  
#
# Before running the tests the vobox must be registered into the 
# MyProxy server.
#
# USAGE: lcg_vobox_test
# Authors, bugs, questions: patricia.mendez@cern.ch
#                           pablo.saiz@cern.ch
#                           catalin.cirstoiu@cern.ch
#			                      vikas.singhal@cern.ch
#
# Adapted for GLite certification by gianni.pucciani@cern.ch
#
###################################################################

use strict;
#use warnings;

use Net::Domain qw(hostname hostfqdn hostdomain);
use Net::LDAP;
use Net::LDAP::Entry;


### Getting the arguments from command line ########

my $node;
my @nodes;

### Creating Timestamp to put with each test.
my $TimeStamp = `date '+%y%m%d%H%M%S'`;
chop $TimeStamp;
print $TimeStamp;


for (my $i = 0; $i<=$#ARGV; $i++){   
    if ($ARGV[$i] =~ /^-node$/) {
        for(my $j = $i + 1; $j <= $#ARGV; $j++) {                                                                                                      
            last if ($ARGV[$j] =~ /^-/);
            push (@nodes,$ARGV[$j]);
            $i = $j-1;
	    
        }
        $node = "@nodes";
    }
}

#### Making the subdirectories ####################
if (!-e "/tmp/$node"){
#    print "$node\n";
    mkdir "/tmp/$node", 0755 or die "directory cannot be created";
}

### helper functions ################################

sub dumpStatus {
    my $service = shift;
    my $status = shift;
    my $message = shift;
    my %others = @_;
    $others{"Message"} = $message if($message);
    my $extra = "";
    while(my ($key, $value) = each(%others)){
        $extra .= "\t$key\t$value";
    }
    print "$service\tStatus\t$status".$extra."\n";
}

# take only the last 5 lines of the given string and concatenate them;
# remove any tabs
sub filter_out {
    my $text = shift;
    
    $text =~ s/\t/ /g;
    my @lines=split(/\n/, $text);
    @lines = @lines[@lines-5 .. @lines-1] if(@lines > 5);
    return join(" ", @lines);
}

sub parse_proxy_timeleft {
    my $proxy_file = shift;
    my $command = shift;
    my $proxy_type = shift;

    my $proxy_out = `$command 2>&1`;
    return (1, "Failed to execute '$command': ".filter_out($proxy_out), 0) if ($?);
    
    my $leftt = $1 if $proxy_out =~ /timeleft\s*:\s*([0-9:]*)/g;
    my $timeleft = 0;
    my $err = 0;
    my $msg = undef;
    if($leftt){
            my @leftl = reverse(split(/:/, $leftt));
            $timeleft = ($leftl[0] || 0) + 60 * ($leftl[1] || 0) + 3600 * ($leftl[2] || 0);
    }else{
            $msg = ($proxy_file ?
                    (-r $proxy_file ?
                            "$proxy_type doesn't point to a readable file."
                            :
                            "Failed checking the proxy for $proxy_type.")
                    :
                    "Undefined proxy file for $proxy_type.");
            $err = 1;
    }
    return ($err, $msg, $timeleft);
} 

my $currentpath = $ENV{PWD};
#####################################################
# 1st TEST:
# Checking if the proxy renewal service is running
######################################################
sub test1 {
    my $service = "Proxy Renewal";
    my $flag = 0;

    system("rm -f /tmp/$node/vobox-test-1.def");
    open VOBOX, ">>/tmp/$node/vobox-test-1.def";
    print VOBOX "testName: VOBOX-Proxy-Renewal\n";
    print VOBOX "testAbbr: PR\n";
#    print VOBOX "testTitle: Status of the alice-box-proxy renew service\n";
    print VOBOX "testTitle: Status of the dteam-box-proxy renew service\n";
    close VOBOX;

    system("rm -f /tmp/$node/vobox-test-1.env");
    open VOBOX, ">>/tmp/$node/vobox-test-1.env";
    print VOBOX "envName: VOBOX-$TimeStamp\n";
    print VOBOX "name: dummy\n";
    print VOBOX "value: none\n";
    close VOBOX;

#    my $rez = `/etc/init.d/alice-box-proxyrenewal status 2>&1` || "Failed to execute /etc/init.d/alice-box-proxyrenewal status";
    my $rez = `/etc/init.d/dteam-box-proxyrenewal status 2>&1` || "Failed to execute /etc/init.d/dteam-box-proxyrenewal status";
    
    if($rez =~ /Service\s+not\s+running/){
	$flag = 1;
    }elsif($rez =~ /Service\s+running\s+in\s+pid/){
	$flag = 0;
    }else{
	$rez =~ s/\n/ /g;
	$flag = 2;
    }

    system("rm -f /tmp/$node/vobox-test-1.result");
    open VOBOX, ">>/tmp/$node/vobox-test-1.result";
    print VOBOX "nodeName: $node\n";
    print VOBOX "testName: VOBOX-Proxy-Renewal\n";
    print VOBOX "envName: VOBOX-$TimeStamp\n";
    print VOBOX "voName: dteam\n";
    if ($flag ==1){
	print VOBOX "status: 50\n";
    }
    if ($flag ==0){
	print VOBOX "status: 10\n";
    }
    if ($flag ==2){
	print VOBOX "status: 40\n";
    }
    print VOBOX "detailedData: EOT\n";
    print VOBOX "<br>\n";
    if ($flag ==1){
#	print VOBOX "Failed to execute /etc/init.d/alice-box-proxyrenewal status\n
	print VOBOX "Failed to execute /etc/init.d/dteam-box-proxyrenewal status\n
		     Check the status of the service dteam-box-proxyrenewal and eventually restart it\n";
    }
    if ($flag ==0){
	print VOBOX "Proxy renewal service (/etc/init.d/dteam-box-proxyrenewal) is up and running\n";
    }
    if ($flag ==2){
	print VOBOX "$rez\n";
    }
    print VOBOX "<br>\n";
    print VOBOX "EOT\n";
    close VOBOX;

}

#####################################################
# 2nd TEST:
# Checking the proxy registration
#####################################################
sub test2 {
    my $service = "User Proxy Registration";

    system("rm -f /tmp/$node/vobox-test-2.def");
    open VOBOX, ">>/tmp/$node/vobox-test-2.def";
    print VOBOX "testName: VOBOX-User-Proxy-Registration\n";
    print VOBOX "testAbbr: UPR\n";
    print VOBOX "testTitle: Status of the user proxy registration\n";
    close VOBOX;

    system("rm -f /tmp/$node/vobox-test-2.env");
    open VOBOX, ">>/tmp/$node/vobox-test-2.env";
    print VOBOX "envName: VOBOX-$TimeStamp\n";
    print VOBOX "name: dummy\n";
    print VOBOX "value: none\n";
    close VOBOX;

    open VOBOX, ">>/tmp/$node/vobox-test-2.result";
    print VOBOX "nodeName: $node\n";
    print VOBOX "testName: VOBOX-User-Proxy-Registration\n";
    print VOBOX "envName: VOBOX-$TimeStamp\n";
    print VOBOX "voName: dteam\n";
    
    my $proxy_registration=`vobox-proxy --vo dteam --force register 2>&1`;
    
    if ($proxy_registration=~ /successfull/ || $proxy_registration=~ /OK/){
	print VOBOX "status: 10\n";
	print VOBOX "detailedData: EOT\n";
	print VOBOX "<br>\n";	
	print VOBOX "User Proxy Registration mechanism is up and working\n";
	print VOBOX "<br>\n";
    	print VOBOX "EOT\n";
    	close VOBOX;	
    }else{

	print VOBOX "status: 50\n";
	print VOBOX "detailedData: EOT\n";
	print VOBOX "<br>\n";	
	print VOBOX "The User Proxy Registration not working. Failed to execute vobox-proxy --vo dteam register --force
		     The User is not allowed to register his proxy within the VOBOX\n";
	print VOBOX "<br>\n";
    	print VOBOX "EOT\n";
    	close VOBOX;
    }    
}

######################################################################
# 3rd TEST:
# is the machine properly registered inside the myproxy server?
######################################################################
sub test3 {
    my $service = "Proxy Server";
    my $command = "env X509_USER_CERT=$ENV{X509_USER_PROXY} X509_USER_KEY=$ENV{X509_USER_PROXY} myproxy-info -d 2>/dev/null";
    my ($err, $msg, $timeleft) = parse_proxy_timeleft($ENV{X509_USER_PROXY}, $command, "X509_USER_CERT/KEY/PROXY for MyProxy");
    
    system("rm -f /tmp/$node/vobox-test-3.def");
    open VOBOX, ">>/tmp/$node/vobox-test-3.def";
    print VOBOX "testName: VOBOX-Proxy-Server-Registration\n";
    print VOBOX "testAbbr: PSR\n";
    print VOBOX "testTitle: Status of the Proxy Server Registration\n";
    close VOBOX;
    
    system("rm -f /tmp/$node/vobox-test-3.env");
    open VOBOX, ">>/tmp/$node/vobox-test-3.env";
    print VOBOX "envName: VOBOX-$TimeStamp\n";
    print VOBOX "name: dummy\n";
    print VOBOX "value: none\n";
    close VOBOX;
    
    system("rm -f /tmp/$node/vobox-test-3.result");
    open VOBOX, ">>/tmp/$node/vobox-test-3.result";
    print VOBOX "nodeName: $node\n";
    print VOBOX "testName: VOBOX-Proxy-Server-Registration\n";
    print VOBOX "envName: VOBOX-$TimeStamp\n";
    print VOBOX "voName: dteam\n";
    
    
    my $proxy_server = $ENV{MYPROXY_SERVER};
    if (!$proxy_server){
	
	print VOBOX "status: 50\n";
	print VOBOX "detailedData: EOT\n";
	print VOBOX "<br>\n";
	print VOBOX "The myproxy server is not defined. Check the corresponding env variable MYPROXY\_SERVER
		     This variable must be pointing to myproxy.cern.ch\n";
	print VOBOX "<br>\n";
	print VOBOX "EOT\n";
	close VOBOX;
	
    }else{
	my $query = "lxbra2305.cern.ch:2170";
	my $host = hostfqdn();
	my $base="o=grid";
	my (%test_hash, @elemento1, @elemento2);
#	my $querySer = "Query to the proxy server";
#	my $connSer = "Connection to the proxy server";
#	my $regSer = "Registration of the VOBOX in the myproxy server";

	if (my $ldap = Net::LDAP->new($query)){

	    if ($ldap->bind){

		my $mesg = $ldap->search( base   => "$base",
					  filter => "objectClass=GlueTop"
					  );
		
		my $total = $mesg->count;
		
		my @values=$mesg->entry(0)->get_value("GlueServiceAccessControlRule");
		my $tmp_vals = @values;
		
		$ldap->unbind;
		
		for ( my $t = 0 ; $t < $tmp_vals ; $t++ ) {
		    $elemento1[$t] = $values[$t];
		    $elemento2[$t] = $t;
		    
		    $test_hash{$elemento1[$t]} = $elemento2[$t];
		}
				
		foreach (sort keys %test_hash){
		    if (/$host/){
			print VOBOX "status: 10\n";
			print VOBOX "detailedData: EOT\n";
			print VOBOX "<br>\n";
			print VOBOX "Registration of the VOBOX in the myproxy server working fine\n";
#			print VOBOX "$service, $err, $msg, 'timeleft' => $timeleft\n";
			print VOBOX "<br>\n";
			print VOBOX "EOT\n";
			close VOBOX;
		    }
		    else {
			print VOBOX "status: 50\n";
			print VOBOX "detailedData: EOT\n";
			print VOBOX "<br>\n";
			print VOBOX "This node is not registered in the myproxy server.
				     Make a query to that server using:
				     ldapsearch -p 2135 -h myproxy.cern.ch -x -LLL -b \"mds-vo-name=local,o=grid\"
				     and check that the DN of your machine appears in this server\n";
#			print VOBOX "$service, $err, $msg, 'timeleft' => $timeleft\n";
			print VOBOX "<br>\n";
			print VOBOX "EOT\n";
			close VOBOX;
		    }
		}
	    }else{
		print VOBOX "status: 50\n";
		print VOBOX "detailedData: EOT\n";
		print VOBOX "<br>\n";
		print VOBOX "BDII server [ $query ] does not respond to the connection\n";
#		print VOBOX "$service, $err, $msg, 'timeleft' => $timeleft\n";
		print VOBOX "<br>\n";
		print VOBOX "EOT\n";
		close VOBOX;
	    }
	}else{
	    print VOBOX "status: 50\n";
	    print VOBOX "detailedData: EOT\n";
	    print VOBOX "<br>\n";
	    print VOBOX "BDII server [ $query ] does not respond to the connection\n";
	    print VOBOX "$service, $err, $msg, 'timeleft' => $timeleft\n";
	    print VOBOX "<br>\n";
	    print VOBOX "EOT\n";
	    close VOBOX;
	}

	
#	if ($err == 0){
#	    $err = 10;
#	}
#	else{
#	    $err = 50;
#	}

	print VOBOX "status: 10\n";
	print VOBOX "detailedData: EOT\n";
	print VOBOX "<br>\n";
	print VOBOX "succesfull test: The machine is properly defined within myproxy server and connections with BDII
                     are properly established\n";
	print VOBOX "<br>\n";
	print VOBOX "EOT\n";
	close VOBOX;
	
    }
}

##################################################
# 4th TEST:
# is the machine running its proxy?
##################################################
sub test4 {
    my $service = "Proxy of the machine";

    system("rm -f /tmp/$node/vobox-test-4.def");
    open VOBOX, ">>/tmp/$node/vobox-test-4.def";
    print VOBOX "testName: VOBOX-Proxy-of-the-machine\n";
    print VOBOX "testAbbr: PM\n";
    print VOBOX "testTitle: Status of the Proxy of the machine\n";
    close VOBOX;

    system("rm -f /tmp/$node/vobox-test-4.env");
    open VOBOX, ">>/tmp/$node/vobox-test-4.env";
    print VOBOX "envName: VOBOX-$TimeStamp\n";
    print VOBOX "name: dummy\n";
    print VOBOX "value: none\n";
    close VOBOX;

    system("rm -f /tmp/$node/vobox-test-4.result");
    open VOBOX, ">>/tmp/$node/vobox-test-4.result";
    print VOBOX "nodeName: $node\n";
    print VOBOX "testName: VOBOX-Proxy-of-the-machine\n";
    print VOBOX "envName: VOBOX-$TimeStamp\n";
    print VOBOX "voName: dteam\n";
    


    if(! -d "/opt/vobox/dteam"){

	print VOBOX "status: 50\n";
	print VOBOX "detailedData: EOT\n";
	print VOBOX "<br>\n";
	print VOBOX "Directory /opt/vobox/dteam does not exist\n";
	print VOBOX "<br>\n";
	print VOBOX "EOT\n";
	close VOBOX;
    }else{
	my $proxy_file = "/opt/vobox/dteam/renewal-proxy.pem";
	my $command = "grid-proxy-info -f $proxy_file 2>/dev/null";
	my ($err, $msg, $timeleft) = parse_proxy_timeleft($proxy_file, $command, "renewal-proxy.pem");
	if ($err == 0){
	    $err = 10;
	    print VOBOX "status: $err\n";
	    print VOBOX "detailedData: EOT\n";
	    print VOBOX "<br>\n";
#	    print VOBOX "$service, $err, $msg, 'timeleft' => $timeleft\n";
	    print VOBOX "The proxy of the machine is up and running\n";
	    print VOBOX "<br>\n";
	    print VOBOX "EOT\n";
	close VOBOX;
	}
	else{
	    $err = 50;
	    print VOBOX "status: $err\n";
	    print VOBOX "detailedData: EOT\n";
	    print VOBOX "<br>\n";
	    print VOBOX "The proxy of the machine is not running
                         Check: grid-proxy-info -f /opt/vobox/dteam/renewal-proxy.pem with an sgm account\n";
	    print VOBOX "<br>\n";
	    print VOBOX "EOT\n";
	    close VOBOX;
	}

#	print "$err\n";
	
#	print VOBOX "status: $err\n";
#	print VOBOX "detailedData: EOT\n";
#	print VOBOX "<br>\n";
#	print VOBOX "$service, $err, $msg, 'timeleft' => $timeleft\n";
#	print VOBOX "<br>\n";
#	print VOBOX "EOT\n";
#	close VOBOX;
    }
}

##################################################
# 5th TEST:
# what is the duration of delegation proxy we can get
##################################################
sub test5 {
   my $service = "Delegated proxy";
   my $out;

   system("rm -f /tmp/$node/vobox-test-5.def");
   open VOBOX, ">>/tmp/$node/vobox-test-5.def";
   print VOBOX "testName: VOBOX-Delegated-proxy-duration\n";
   print VOBOX "testAbbr: DPD\n";
   print VOBOX "testTitle: Status of the Proxy of the machine\n";
   close VOBOX;
   
   system("rm -f /tmp/$node/vobox-test-5.env");
   open VOBOX, ">>/tmp/$node/vobox-test-5.env";
   print VOBOX "envName: VOBOX-$TimeStamp\n";
   print VOBOX "name: dummy\n";
   print VOBOX "value: none\n";
   close VOBOX;
   
   system("rm -f /tmp/$node/vobox-test-5.result");
   open VOBOX, ">>/tmp/$node/vobox-test-5.result";
   print VOBOX "nodeName: $node\n";
   print VOBOX "testName: VOBOX-Delegated-proxy-duration\n";
   print VOBOX "envName: VOBOX-$TimeStamp\n";
   print VOBOX "voName: dteam\n";
   
   
   my $renewalProxy = "/opt/vobox/dteam/renewal-proxy.pem";
   my $currentProxy = $ENV{X509_USER_PROXY};
   my $duration = 259_200;	# 3 days
   my $delegatedFile = "/tmp/tmpfile.$$";
 
   if(-r $renewalProxy){

	    my @command = ('vobox-proxy', "--vo", "dteam", "--voms", "dteam:/dteam/Role=lcgadmin", "query");
	    $out = `@command`;
      print "$out\n";	    
    if($?){
		print VOBOX "status: 50\n";
		print VOBOX "detailedData: EOT\n";
		print VOBOX "<br>\n";
		print VOBOX "Failed to check the timeleft for the user delegated proxy\n";
		print VOBOX "the command: vobox-proxy --vo dteam --voms dteam:/dteam/Role=lcgadmin query failed\n";
#		print VOBOX "filter_out($out), 'timeleft' => 0\n";
		print VOBOX "<br>\n";
		print VOBOX "EOT\n";
		close VOBOX;
	    }else{
	       if($out =~ /\s*(\d+)\s*/){
		   print VOBOX "status: 10\n";
		   print VOBOX "detailedData: EOT\n";
		   print VOBOX "<br>\n";
		   print VOBOX "The delegation procedure is working properly: 'timeleft' => $1\n";
		   print VOBOX "<br>\n";
		   print VOBOX "EOT\n";
		   close VOBOX;

	       }else{
#		  dumpStatus($service, 1, "Failed to parse timeleft ".filter_out($out), 'timeleft' => 0);
		   print VOBOX "status: 40\n";
		   print VOBOX "detailedData: EOT\n";
		   print VOBOX "<br>\n";
		   print VOBOX "Not possible to parse the timeleft. This is a warning, please check the tests PR and PSR\n";
		   print VOBOX "filter_out($out), 'timeleft' => 0\n";
		   print VOBOX "<br>\n";
		   print VOBOX "EOT\n";
		   close VOBOX;
	       }
	   }
	}

}
#	 unlink($delegatedFile);
#	 $ENV{X509_USER_PROXY} = $currentProxy;
#     }else{
##	 dumpStatus($service, 1, 'Cannot read USER_PROXY', 'timeleft' => 0);
#	 print VOBOX "status: 50\n";
#	 print VOBOX "detailedData: EOT\n";
#	 print VOBOX "<br>\n";
#	 print VOBOX "Cannot read the variable X509_USER_PROXY. Please make a grid-proxy-info once you get inside the VOBOX\n";
##	 print VOBOX "filter_out($out), 'timeleft' => 0\n";
#	 print VOBOX "<br>\n";
#	 print VOBOX "EOT\n";
#	 close VOBOX; 
#     }
#  }else{
##      dumpStatus($service, 1, 'Cannot read renewal_proxy.pem', 'timeleft' => 0);
#      print VOBOX "status: 50\n";
#      print VOBOX "detailedData: EOT\n";
#      print VOBOX "<br>\n";
#      print VOBOX "The file: /opt/vobox/alice/renewal-proxy.pem containing the machine proxy is not readable\n";
#      print VOBOX "filter_out($out), 'timeleft' => 0\n";
#      print VOBOX "<br>\n";
#      print VOBOX "EOT\n";
#      close VOBOX; 
#  }
#}



################################################################
# 6th TEST:
# Check if the access to the software area is properly defined for dteamsgm persons
################################################################

sub test6 {
    my $service = "Software area";

    system("rm -f /tmp/$node/vobox-test-6.def");
    open VOBOX, ">>/tmp/$node/vobox-test-6.def";
    print VOBOX "testName: VOBOX-Software-area\n";
    print VOBOX "testAbbr: SA\n";
    print VOBOX "testTitle: Status of the Software area access\n";
    close VOBOX;
    
    system("rm -f /tmp/$node/vobox-test-6.env");
    open VOBOX, ">>/tmp/$node/vobox-test-6.env";
    print VOBOX "envName: VOBOX-$TimeStamp\n";
    print VOBOX "name: dummy\n";
    print VOBOX "value: none\n";
    close VOBOX;
    
    system("rm -f /tmp/$node/vobox-test-6.result");
    open VOBOX, ">>/tmp/$node/vobox-test-6.result";
    print VOBOX "nodeName: $node\n";
    print VOBOX "testName: VOBOX-Software-area\n";
    print VOBOX "envName: VOBOX-$TimeStamp\n";
    print VOBOX "voName: dteam\n";
    

    my $software_area = $ENV{'VO_DTEAM_SW_DIR'};

     if (! -e $software_area){	
            print VOBOX "status: 50\n";
            print VOBOX "detailedData: EOT\n";
            print VOBOX "<br>\n";
            print VOBOX "The software area variable: VO\_DTEAM\_SW\_DIR is not defined \n";
            print VOBOX "<br>\n";
            print VOBOX "EOT\n";
            close VOBOX;

    }else{
	chdir "$ENV{VO_DTEAM_SW_DIR}> /dev/null 2> /dev/null";
	if($?){
	    print VOBOX "status: 50\n";
	    print VOBOX "detailedData: EOT\n";
	    print VOBOX "<br>\n";
	    print VOBOX "Permission denied within the software area. A sgm user cannot get into this region\n";
	    print VOBOX "<br>\n";
	    print VOBOX "EOT\n";
	    close VOBOX; 
	}
	else{
	    system ("touch filetest$$");
	    if($?){
	        print VOBOX "status: 50\n";
	        print VOBOX "detailedData: EOT\n";
	        print VOBOX "<br>\n";
	        print VOBOX "Write access interdit within the software area for sgm accounts\n";
	        print VOBOX "<br>\n";
	        print VOBOX "EOT\n";
	        close VOBOX; 
	    }else{
		system ("rm filetest$$");
		print VOBOX "status: 10\n";
		print VOBOX "detailedData: EOT\n";
		print VOBOX "<br>\n";
		print VOBOX "The access to the software area is guaranteed \n";
		print VOBOX "<br>\n";
		print VOBOX "EOT\n";
		close VOBOX; 
	    }
	}
    }
}

#################################################################
for(my $i = 1; $i < 7; $i++){
    eval "test$i()";
}

dumpStatus("SCRIPTRESULT", 0);

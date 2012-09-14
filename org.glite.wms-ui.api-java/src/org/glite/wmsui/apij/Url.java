/*
* Url.java
*
* Copyright (c) 2001 The European DataGrid Project - IST programme, all rights reserved.
*
* Contributors are mentioned in the code there appropriate.
*
*/
package org.glite.wmsui.apij;
import java.lang.* ;
import java.net.URL ;
/**
 * Managing Addresses
 * File name: Url.java
 *
 * @version 0.1
 * @author Alessandro Maraschini <alessandro.maraschini@datamat.it>
*/

public class Url {
	/**  constructor by String
	* Parse the string in order to create a valid Url
	* @param address the full address in the form [<protocol>://]<host>:<port>
	*/
	public Url (String address)  throws NumberFormatException , IllegalArgumentException {
		setAddress (address);
	}
	/**  constructor by parameters
	* Parse Initialise the host and the port of the Url
	* @param host the host address of this Url
	* @param port the integer port of this Url
	*/
	public Url (String host , int port) {
		this.host = host ;
		this.port = port  ;
		address = host +":" + String.valueOf (port) ;
	}
	/**
	* Convert the Url Intance into its string representation */
	public String toString () {
		if (address!= null) return address ;
		else return null ;
	}
	/**********************************
	* Get Methods
	*********************************/
	/**
	* Retrieve the complete Url address
	*@return the full address of this Url
	*/
	public String getAddress(){ return address; }
	/**
	* Retrieve the port
	*@return the port of this Url
	*/
	public int getPort() { return port ; }
	/**
	* Retrieve the host
	*@return the host with its protocol if present
	*/
	public String getHost() { return host ; }
	/**
	* Retrieve the simple host name whithout any additiona info (protocol)
	*@return the host without its protocol (if is present)
	*/
	public String getSimpleHost(){
		int proto =  address.lastIndexOf    ("://")   ;
		if ( proto<0 )    //   Protocol not found:
			return host ;
		else
			return host.substring(proto+3, host.length() );
	}
	/**
	* Retrieve the  protocol if present, null otherwise
	*@return the  protocol if present, null otherwise
	*/
	public String getProtocol() {
		int proto =  address.lastIndexOf    ("://")   ;
		if ( proto<0 )  //   Protocol not found:
			return null ;
		else
			return host.substring(0, proto+3 );
	}
	/**  Check Wheater this Url could be a valid NetworkServer address */
	void checkNS() throws IllegalArgumentException {
		// checkNS(host, port) ;
		int protocol =  host.indexOf    (":")   ;
		if(        (protocol>=0)  || (port<1024)     ){
			throw  new IllegalArgumentException ("Bad NS format: <host address>:<port number>")  ;
		}
	}
	/**  Set The Address */
	private void setAddress (String address) throws NumberFormatException , IllegalArgumentException {
		if ( address.trim().length() == 0)
			throw new IllegalArgumentException ("Unable to parse: Empty string address");
		int sep =  address.lastIndexOf    (":")   ;
		if (sep<=0){
			throw new IllegalArgumentException ("Bad Url format: "+ address+"\nRight format is:  <host address>:<port number>")    ;
		}
		host = address.substring(0, sep)  ;           //Host
		port =java.lang.Integer.parseInt(  address.substring  (sep+1, address.length()   )  )  ;  //Port
		this.address = address ;
	}

	/**Private Members:*/
	private String address ;
	private String host;
	private int port ;


};

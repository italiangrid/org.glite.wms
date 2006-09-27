/*
 * Copyright (c) Members of the EGEE Collaboration. 2004.
 * See http://eu-egee.org/partners/ for details on the copyright holders.
 * For license conditions see the license file or http://eu-egee.org/license.html
 */

package org.glite.wms.wmproxy.proxyinfo ;

// WMP-API-JAVA
import org.glite.wms.wmproxy.ProxyInfoStructType;
import org.glite.wms.wmproxy.VOProxyInfoStructType;

import java.lang.NullPointerException;
import java.lang.Long;
import java.util.Calendar ;
import java.util.TimeZone ;
import java.util.Date;

import org.bouncycastle.asn1.DERGeneralizedTime;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.SimpleTimeZone;
//importjava.text.DateFormat;
/**
public ProxyInfoStructType(
           java.lang.String subject,
           java.lang.String issuer,
           java.lang.String identity,
           java.lang.String type,
           java.lang.String strength,
           java.lang.String startTime,
           java.lang.String endTime,
           org.glite.wms.wmproxy.VOProxyInfoStructType[] VOsInfo) {
           this.subject = subject;
           this.issuer = issuer;
           this.identity = identity;
           this.type = type;
           this.strength = strength;
           this.startTime = startTime;
           this.endTime = endTime;
           this.VOsInfo = VOsInfo;
    }
*/
public class ProxyInfo  {

	public ProxyInfo  ( ) {}

	private int getSpaces(String label, String value) {
		return( LINE_LENGTH - label.length( ));
	}
	private String getAttribute(String label, String value)throws NullPointerException {
		String line = "" ;
		if (value != null && value.length()>0){
			line += label + ": ";
			int ws = getSpaces(label, value);
			// Adds "ws" white spaces
			for (int i=0; i < ws; i++){ line += " ";}
			line += value ;
		} else {
			throw new NullPointerException ("no value for " + label );
		}
		return line;
	}

	private String twodigits(int n) {
		String td = "";
		if (n>=0 && n<10) {
			td = "0" + n ;
		} else {
			td = "" + n;
		}
		return td;
	}

	private String convTime(String time) {

		String date = "";
		int hours = 0;
		Long t = new Long(time);
		long value = t.longValue( )  * 1000;
		Calendar calendar = Calendar.getInstance();
		// time
		calendar.setTime(new Date(value));
		// time-zone
		calendar.setTimeZone(java.util.TimeZone.getTimeZone("GMT"));
		// conversion to string
		date = dayStr[(calendar.get(Calendar.DAY_OF_WEEK)-1)] + " " +
			monthStr[calendar.get(Calendar.MONTH)] + " " +
			twodigits(calendar.get(Calendar.DAY_OF_MONTH)) + " "
			+ calendar.get(Calendar.YEAR) + " ";
		date += twodigits(calendar.get(Calendar.HOUR_OF_DAY)) + ":" +
			twodigits(calendar.get(Calendar.MINUTE)) + ":" +
			twodigits(calendar.get(Calendar.SECOND)) ;
				date += " " + calendar.getTimeZone().getID( );
		return date ;

	}

	public String getProxyInfo(ProxyInfoStructType infoStruct ) throws NullPointerException, ParseException  {
		String info = "";
		String[] attrs;
		String fqan = "";
		int size = 0;
		int nattrs = 0;
		if ( infoStruct != null)  {
			try  { info += getAttribute ("Subject", infoStruct.getSubject()) + "\n";} catch (NullPointerException e ){}
			try  { info += getAttribute ("Issuer", infoStruct.getSubject()) + "\n";} catch (NullPointerException e ){}
			try  { info += getAttribute ("Identity", infoStruct.getIdentity()) + "\n";} catch (NullPointerException e ){}
			try  { info += getAttribute ("Type", infoStruct.getType()) + "\n";} catch (NullPointerException e ){}
			try  { info += getAttribute ("Strength", infoStruct.getStrength()) + "\n";} catch (NullPointerException e ){}
			try  { info += getAttribute ("StartTime", convTime(infoStruct.getStartTime())) + "\n";} catch (NullPointerException e ){}
			try  { info += getAttribute ("EndTime", convTime(infoStruct.getEndTime())) + "\n";} catch (NullPointerException e ){}
			VOProxyInfoStructType[] vos = infoStruct.getVOsInfo() ;
			size = vos.length;
			for (int i = 0; i < size ; i++) {
				try  { info += "=== VO "+ vos[i].getVOName() + " extension information ===\n";} catch (NullPointerException e ){}
				try  { info += getAttribute ("Subject" , vos[i].getUser() )+ "\n"; } catch (NullPointerException e ){}
				try  { info += getAttribute ("UserCA" , vos[i].getUserCA())+ "\n"; } catch (NullPointerException e ){}

				try  { info += getAttribute ("Server" , vos[i].getServer())+ "\n"; } catch (NullPointerException e ){}
				try  { info += getAttribute ("ServerCA" , vos[i].getServerCA())+ "\n"; } catch (NullPointerException e ){}
    				try  { info += getAttribute ("URI" , vos[i].getURI())+ "\n"; } catch (NullPointerException e ){}
				attrs= (String[]) vos[i].getAttribute();
				nattrs = attrs.length;
				for (int j = 0; j < nattrs ; j++) {
					if (attrs[j].length()>0){
						if (attrs[j].substring(0,1)!="/") { fqan += "/";}
						fqan = attrs[j] ;
					}
				}
				try  { info += getAttribute ("FQAN" , fqan)+ "\n"; } catch (NullPointerException e ){}
				try  { info += getAttribute ("StartTime", convTime(vos[i].getStartTime())) + "\n";} catch (NullPointerException e ){}
				try  { info += getAttribute ("EndTime", convTime(vos[i].getEndTime())) + "\n";} catch (NullPointerException e ){}
			}
		} else {
			throw new NullPointerException (" Empty ProxyInfoStructType" );
		}
		return info;
	}
	
	private int LINE_LENGTH = 20;
	private ProxyInfoStructType infoStruct ;
	private final static String[] monthStr  = {"Jan", "Feb", "March", "Apr", "May", "June" ,"July", "Aug", "Sept", "Oct", "Nov", "Dec"};
	private final static String[] dayStr = {"Sun", "Mon", "Tue", "Wedn", "Thu", "Fri" ,"Sat"};
}

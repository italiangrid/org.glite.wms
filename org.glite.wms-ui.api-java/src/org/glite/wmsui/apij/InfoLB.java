/*
* InfoLB.java
*
* Copyright (c) 2001 The European DataGrid Project - IST programme, all rights reserved.
*
* Contributors are mentioned in the code there appropriate.
*
*/
package org.glite.wmsui.apij ;
import java.util.* ;
/**
 * LB Information Class Wrapper
 * This class is not visible to the user, restricted to the package
 * @version 0.1
 * @author Alessandro Maraschini <alessandro.maraschini@datamat.it>
*/
public class InfoLB extends HashMap {
	InfoLB() {} ;
	/**
	Retrieve a specified field
	* @param attrName index int valure corresponding to the field to be retrieved
	* @return An Object representing the requested value
	*/
	public Object get(int attrName){
		return super.get(new Integer( attrName  )) ;
	}
	/** Insert the value (create a vector when needed) 
	*@param attrName the name of the attribute to be put
	*@param attrValue an object corresponding to the value of the attribute to be put
	*/
	void put (int attrName , Object attrValue) {
		Object obj = get( attrName) ;
		if (obj==null){  // it is a single value
			super.put( new Integer (attrName)  , attrValue) ;
		}else if ( obj instanceof Vector ){
			// Is already a vector: append value
			((Vector)obj).add (attrValue) ;
		}else {
			// Is the second value, create the vector
			Vector vect = new Vector() ;
			vect.add (obj) ;
			vect.add (attrValue) ;
			super.put( new Integer (attrName) , vect ) ;
		}
	}
	/**  Retrieve the integer value of the specified attribute
	*@param i the attribute to be retrieved
	*@return the int value corresponding to the requested attribute
	*@throws java.lang.ClassCastException  */
	public int getValInt (int i)  {
		return  ((Integer) get(i)).intValue()  ;
	}
	/**  Retrieve the String value of the specified attribute
	*@return the String value corresponding to the requested attribute
	*@throws java.lang.ClassCastException  */
	public String getValString ( int i )  {return  (String) get (i) ; }

	/** Convert the instance into a String 
	*@return the string representation of LB info
	*/
	public String toString (){
		String result = "" ;
		for (int i = 0 ; i< size() ; i++ ) {
			Object obj =get(i);
			if ( obj!= null ) result += obj.toString() + "\n" ;
			else result += "NULL\n"  ;
		}
		return result ;
	}
	/***************************************
	* Public int members:
	***************************************/
	/** Verbosity Level: only basic information*/
	public static int DEFAULT_LOG_LEVEL = 0 ;
	/** Verbosity Level: Normal Information, only CLASSAD info are not retrieved*/
	public static int NORMAL_LOG_LEVEL = 1 ;
	/** Verbosity Level: Complete information, full attributes and CLASSAD info retrieved */
	public static int HIGH_LOG_LEVEL       = 2 ;
} // end class


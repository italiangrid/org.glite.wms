// File: step.h
// Author: Alessio Gianelle <gianelle@pd.infn.it>
// Copyright (c) 2002 EU DataGrid.
// For license conditions see http://www.eu-datagrid.org/license.html

// $Id$

/**
 * \file
 * \brief Provides the Step Object. 
 *
 * \version 0.1
 * \date 18 December 2002
 * \author Alessio Gianelle <gianelle@pd.infn.it>
*/

#ifndef EDG_WORKLOAD_CHECKPOINTING_STEP_H
#define EDG_WORKLOAD_CHECKPOINTING_STEP_H

#include <glite/wms/checkpointing/checkpointing_namespace.h>
#include <string>

CHKPT_NAMESPACE_BEGIN { 

  /**      
   * \brief Provides the label of a single step.
   */
  class Step {
    
  public:
    /** Provides the type of the label. */
    enum step_type { 
      integer,  /**< Numeric label. */
      label     /**< String label. */
    };
    /** \name  Constructors Destructor */ 
    /// @{ 
    /** Instantiates a Step object from a numeric label.
	\param istep - An int representing the label. */	
    Step( int istep );
    /** Instantiates a Step object from a string label.
	\param sstep - A string representing the label. */
    Step( const std::string &sstep );
    /** Instantiates a Step object from a char* label.
	\param sstep - A char* representing the label. */
    Step( const char *sstep ); 
    Step( const Step &ss ); /**< The copy constructor method. */
    ~Step( void ); /**< The Destructor method. */ 
    /// @}  
    
    /** \name Check-Type Methods. */
    /// @{
    /** Check if the object represents a string label.
	\return @a true if the label is a string value; @a false otherwise. */ 
    inline bool isLabel( void ) { return( this->s_type == label ); }
    /** Check if the object represents a numeric label.
	\return @a true if the label is a numeric value; @a false otherwise. */ 
    inline bool isInteger( void ) { return( this->s_type == integer ); }
    /// @}
    
    /** \name Get Methods. */
    /// @{
    /** Extracts the value of the numeric label.
	\return an int with the value of the label. 
	\throws WTException */
    int getInteger( void );
    /** extracts the value of the string label.
	\return a string with the value of the label. 
	\throws WTException */
    const std::string &getLabel( void ); 
    /// @}    

    /** \name Operators */
    /// @{
    /** Copy operator.
     	\param that - The Step object to copy.
	\return The current object. */
    Step &operator=( const Step &that );
    /** Int cast operator. 
	\return an int with the value of the label. 
	\throws WTException */
    inline operator int( void ) { return ( getInteger() ); }
     /** String cast operator.
	\return a string with the value of the label. 
	\throws WTException */
    inline operator const std::string &( void ) { return( getLabel() ); }
    /// @}

  private:
    step_type       s_type;     /**< The type of the step label. */ 
    union {                     
      int           s_u_istep;  /**< Numeric label. */
      std::string  *s_u_sstep;  /**< String label. */
    };    
  };

} CHKPT_NAMESPACE_END;

#endif // EDG_WORKLOAD_CHECKPOINTING_STEP_H

//  Local Variables:
//  mode: c++
//  End:

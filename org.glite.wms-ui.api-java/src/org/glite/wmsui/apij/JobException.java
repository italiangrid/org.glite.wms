/*
* JobException.java
*
* Copyright (c) 2001 The European DataGrid Project - IST programme, all rights reserved.
*
* Contributors are mentioned in the code there appropriate.
*
*/
package org.glite.wmsui.apij;
/**
 * Manage the Exception raised when trying to perform an action not allowed for a job such as submitting a jobId-type job
 * or retrieving status information from a non-submitted job ... etc etc
 *
 * @version 0.1
 * @author Alessandro Maraschini <alessandro.maraschini@datamat.it>
*/
public class JobException extends Exception {
    /**
     * The possibly null root cause exception.
     * @serial
     */
    private Throwable exception;

    /**
     * Constructs a new instance of <tt>JobException</tt>.
     * The root exception and the detailed message are null.
     */
    public JobException () {
        super();
    }

    /**
     * Constructs a new instance of <tt>JobException</tt> with a detailed message.
     * The root exception is null.
     * @param detail A possibly null string containing details of the exception.
     *
     * @see java.lang.Throwable#getMessage
     */
    public JobException (String detail) {
        super(detail);
    }

    /**
     * Constructs a new instance of <tt>JobException</tt> with a detailed message
     * and a root exception.
     *
     * @param detail A possibly null string containing details of the exception.
     * @param ex A possibly null root exception that caused this exception.
     *
     * @see java.lang.Throwable#getMessage
     * @see #getException
     */
    public JobException (String detail, Throwable ex) {
        super(detail);
        exception = ex;
    }
    /**
     * Returns the root exception that caused this exception.
     * @return The possibly null root exception that caused this exception.
     */
    public Throwable getException() {
        return exception;
    }
    /**
     * Prints this exception's stack trace to <tt>System.err</tt>.
     * If this exception has a root exception; the stack trace of the
     * root exception is printed to <tt>System.err</tt> instead.
     */
    public void printStackTrace() {
        printStackTrace( System.err );
    }

    /**
     * Prints this exception's stack trace to a print stream.
     * If this exception has a root exception; the stack trace of the
     * root exception is printed to the print stream instead.
     * @param ps The non-null print stream to which to print.
     */
    public void printStackTrace(java.io.PrintStream ps) {
    if ( exception != null ) {
        String superString = super.toString();
        synchronized ( ps ) {
            ps.print(superString
                     + (superString.endsWith(".") ? "" : ".")
                     + "  Root exception is ");
            exception.printStackTrace( ps );
        }
    } else {
        super.printStackTrace( ps );
    }
    }

    /**
     * Prints this exception's stack trace to a print writer.
     * If this exception has a root exception; the stack trace of the
     * root exception is printed to the print writer instead.
     * @param pw The non-null print writer to which to print.
     */
    public void printStackTrace(java.io.PrintWriter pw) {
        if ( exception != null ) {
            String superString = super.toString();
            synchronized (pw) {
                pw.print(superString
                         + (superString.endsWith(".") ? "" : ".")
                         + "  Root exception is ");
                exception.printStackTrace( pw );
            }
        } else {
            super.printStackTrace( pw );
        }
    }

    public String getMessage() {
      String answer = super.getMessage();
      if (exception != null && exception != this) {
            answer += " [Root error message: " + exception.getMessage() + "]";
      }
        return answer;
    }

    /**
     * Returns the string representation of this exception.
     * The string representation contains
     * this exception's class name, its detailed messsage, and if
     * it has a root exception, the string representation of the root
     * exception. This string representation
     * is meant for debugging and not meant to be interpreted
     * programmatically.
     * @return The non-null string representation of this exception.
     * @see java.lang.Throwable#getMessage
     */
    public String toString() {
        String answer = super.toString();
        if (exception != null && exception != this) {
            answer += " [Root exception is " + exception.toString() + "]";
        }
        return answer;
    }
}

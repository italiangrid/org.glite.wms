/*
 * JDLEditorInterface.java
 *
 * Copyright (c) 2001 The European DataGrid Project - IST programme, all rights
 * reserved.
 *
 */

package org.glite.wmsui.guij;


import java.util.Vector;

import java.awt.Component;
import java.awt.event.ActionEvent;

import javax.swing.JButton;
import javax.swing.JMenuBar;

import org.glite.wms.jdlj.JobAd;


/**
 * Implementation of the JDLEditorInterface interface.
 *
 *
 * @ingroup gui
 * @brief
 * @version 1.0
 * @date 8 may 2002
 * @author Giuseppe Avellino <giuseppe.avellino@datamat.it>
 */
public interface JDLEditorInterface {
  JobAd jobAdGlobal = new JobAd();
  //JButton jButtonReset = new JButton();
  //JMenuBar getJMenuBarReference();

  // get, set methods //
  String getUserWorkingDirectory();

  void setUserWorkingDirectory(String directory);

  String getNodeNumberValue();

  //String getJobTypeValue();

  String getHTMLParameter(String paramName);

  void setJTextAreaJDL(String JDLText);

  // view, display methods //
  void viewAll();

  String jButtonDataReqViewEvent(boolean showWarningMsg,
      boolean showErrorMsg, ActionEvent ae);

  void displayJPanelDesktop();

  // enabling methods //
  void setDef1StandardStreamsEnabled(boolean bool);

  void setListenerPortEnabled(boolean bool);

  // exit method //
  void exitApplication();

}

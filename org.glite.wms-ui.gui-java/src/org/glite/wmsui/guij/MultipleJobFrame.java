/*
 * MultipleJobFrame.java
 *
 * Copyright (c) 2001 The European DataGrid Project - IST programme,
 * all rights reserved.
 * Contributors are mentioned in the code where appropriate.
 *
 */

package org.glite.wmsui.guij;


import java.awt.*;
import java.awt.event.*;

import javax.swing.*;
import javax.swing.event.*;

import java.io.*;
import java.util.*;

import org.glite.wms.jdlj.*;
import org.glite.wmsui.apij.*;

import org.apache.log4j.*;


/**
 * Implementation of the MultipleJobFrame class.
 *
 *
 * @ingroup gui
 * @brief Utility class used to store global variables values.
 * @version 1.0
 * @date 8 may 2002
 * @author Giuseppe Avellino <giuseppe.avellino@datamat.it>
 */
public class MultipleJobFrame extends JFrame {
  static Logger logger = Logger.getLogger(GUIUserCredentials.class.getName());

  static final boolean THIS_CLASS_DEBUG = false;
  static boolean isDebugging = THIS_CLASS_DEBUG || Utils.GLOBAL_DEBUG;

  protected JobMonitor jobMonitorFrame;
  protected MultipleJobPanel multipleJobPanel;
  protected String dagJobId;

  /**
   * Constructor.
   */
  public MultipleJobFrame(JobMonitor jobMonitorFrame, String dagJobId,
      String state, String submissionTime) throws Exception,
      IllegalArgumentException {
    super("Job Monitor - VO: " + GUIGlobalVars.getVirtualOrganisation() + " - "
        + "Dag: " + dagJobId);
    this.dagJobId = dagJobId;
    this.jobMonitorFrame = jobMonitorFrame;
    enableEvents(AWTEvent.WINDOW_EVENT_MASK);
    try {
      jbInit(state, submissionTime);
    } catch (IllegalArgumentException iae) {
      throw iae;
    } catch (Exception e) {
      if (isDebugging) {
        e.printStackTrace();
      }
      throw e;
    }
  }

  private void jbInit(String state, String submissionTime) throws Exception,
      IllegalArgumentException {
    isDebugging |= (logger.getRootLogger().getLevel() == Level.DEBUG)
        ? true : false;

    Toolkit toolkit = getToolkit();
    Dimension screenSize = toolkit.getScreenSize();
    this.setSize(new Dimension((int) (screenSize.width
        * GraphicUtils.SCREEN_WIDTH_PROPORTION),
        (int) (screenSize.height * GraphicUtils.SCREEN_HEIGHT_PROPORTION)));

    JobCollection jobCollection = new JobCollection();
    Vector childrenVector = new Vector();
    try {
      childrenVector = (new Job(new JobId(this.dagJobId))).getSubJobsId();
      for (int i = 0; i < childrenVector.size(); i++) {
        jobCollection.insertId(new JobId(childrenVector.get(i).toString()));
      }
    } catch (Exception e) {
      throw e;
    }
    if (childrenVector.size() != 0) {
      try {
        multipleJobPanel = new DagMultipleJobPanel(this, jobMonitorFrame,
            this.dagJobId, state, submissionTime, jobCollection);
        this.getContentPane().add(multipleJobPanel, null);
      } catch (Exception e) {
        throw e;
      }
    } else {
      throw new IllegalArgumentException("Dag '" + this.dagJobId
          + "' has no sub jobs");
    }
  }

  protected void processWindowEvent(WindowEvent e) {
    super.processWindowEvent(e);
    this.setDefaultCloseOperation(DO_NOTHING_ON_CLOSE);
    if (e.getID() == WindowEvent.WINDOW_CLOSING) {
      exit();
    }
  }

  void exit() {
    multipleJobPanel.exitCurrentTimeThread();
    multipleJobPanel.exitUpdateThread();
    this.jobMonitorFrame.dagMonitorMap.remove(this.dagJobId);
    this.dispose();
  }

}
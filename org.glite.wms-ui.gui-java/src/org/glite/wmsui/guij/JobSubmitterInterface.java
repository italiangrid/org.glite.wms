/*
 * JobSubmitterInterface.java
 *
 * Copyright (c) 2001 The European DataGrid Project - IST programme, all rights reserved.

 *
 */


package org.glite.wmsui.guij;


import java.util.Vector;

import java.awt.Component;
import java.awt.event.ActionEvent;

import javax.swing.JButton;
import javax.swing.JMenuBar;

import org.glite.wms.jdlj.Ad;


/**
 * Implementation of the JobSubmitterInterface interface.
 *
 *
 * @ingroup gui
 * @brief
 * @version 1.0
 * @date 8 may 2002
 * @author Giuseppe Avellino <giuseppe.avellino@datamat.it>
 */
public interface JobSubmitterInterface {
  void addJobToTable(String rBName, String keyJobName, String currentOpenedFile, Ad jobAd);
}

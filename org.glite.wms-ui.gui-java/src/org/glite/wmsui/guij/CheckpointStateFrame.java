/*
 * CheckpointStateFrame.java
 *
 * Copyright (c) 2001 The European DataGrid Project - IST programme, all rights reserved.
 * Contributors are mentioned in the code where appropriate.
 *
 */

package org.glite.wmsui.guij;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.border.*;

import org.apache.log4j.*;


/**
 * Implementation of the CheckpointStateFrame class.
 *
 * @ingroup gui
 * @brief
 * @version 1.0
 * @date 8 may 2002
 * @author Giuseppe Avellino <giuseppe.avellino@datamat.it>
 */
public class CheckpointStateFrame extends JFrame {
  static Logger logger = Logger.getLogger(GUIUserCredentials.class.getName());

  static final boolean THIS_CLASS_DEBUG = false;
  static boolean isDebugging = THIS_CLASS_DEBUG || Utils.GLOBAL_DEBUG;

  private String fileName;

  Component component;

  JPanel jPanelButtonEdit = new JPanel();
  JPanel jPanelCheckpointState = new JPanel();
  JButton jButtonEdit = new JButton();
  JScrollPane jScrollPaneCheckpointState = new JScrollPane();
  JTextPane jTextPaneCheckpointState = new JTextPane();


  /**
   * Constructor.
   */
  public CheckpointStateFrame(Component component, String text, String fileName) {
    super("Checkpoint State - " + fileName);
    this.fileName = fileName;
    this.component = component;
    setCheckpointState(text);
    enableEvents(AWTEvent.WINDOW_EVENT_MASK);
    try {
      jbInit();
    }
    catch(Exception e) {
      if (isDebugging) e.printStackTrace();
    }
  }


  private void jbInit() throws Exception  {
    isDebugging |= (logger.getRootLogger().getLevel() == Level.DEBUG) ? true : false;

    setSize(520, 600);
    setResizable(false);
    this.getContentPane().setLayout(new BorderLayout());
    jPanelButtonEdit.setBounds(new Rectangle(0, 0, 700, 50));
    jPanelButtonEdit.setBorder(BorderFactory.createRaisedBevelBorder());
    jPanelCheckpointState.setLayout(new BorderLayout());
    jButtonEdit.setText("    Close    ");
    jButtonEdit.addActionListener(new java.awt.event.ActionListener() {
      public void actionPerformed(ActionEvent e) {
        jButtonCloseEvent(e);
      }
    });
    jTextPaneCheckpointState.setEditable(false);
    jPanelButtonEdit.add(jButtonEdit, null);
    this.getContentPane().add(jPanelCheckpointState, BorderLayout.CENTER);
    jPanelCheckpointState.add(jScrollPaneCheckpointState, BorderLayout.CENTER);
    this.getContentPane().add(jPanelButtonEdit, BorderLayout.SOUTH);
    jScrollPaneCheckpointState.getViewport().add(jTextPaneCheckpointState, null);
    this.getContentPane().setVisible(true);
  }


  public void setCheckpointState(String text) {
    jTextPaneCheckpointState.setText(text);
  }


  void jButtonCloseEvent(ActionEvent e) {
    if (GUIGlobalVars.openedCheckpointStateMap.containsKey(this.fileName)) {
      GUIGlobalVars.openedCheckpointStateMap.remove(this.fileName);
    }
    this.dispose();
  }


  protected void processWindowEvent(WindowEvent e) {
    super.processWindowEvent(e);
    this.setDefaultCloseOperation(DO_NOTHING_ON_CLOSE);
    if (e.getID() == WindowEvent.WINDOW_CLOSING) {
      jButtonCloseEvent(null);
    }
  }


  static void closeAllCheckpointStateFrames() {
    Object[] values = GUIGlobalVars.openedCheckpointStateMap.values().toArray();
    for(int i = 0 ; i < values.length; i++) {
      ((CheckpointStateFrame) values[i]).dispose();
    }
    GUIGlobalVars.openedCheckpointStateMap.clear();
  }



}
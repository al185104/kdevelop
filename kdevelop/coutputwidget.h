/***************************************************************************
                     coutputwidget.h - the output window in kdevelop   
                             -------------------                                         

    version              :                                   
    begin                : 5 Aug 1998                                        
    copyright            : (C) 1998 by Sandy Meier                         
    email                : smeier@rz.uni-potsdam.de                                     
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/
#ifndef COUTPUTWIDGET_H
#define COUTPUTWIDGET_H


//#include "keditcl.h"
#include <qmultilineedit.h>
//#include <qwidget.h>

/** the view for the compiler and tools-output
  *@author Sandy Meier
  */
class COutputWidget : public QMultiLineEdit
{
  Q_OBJECT

public:
  /**contructor*/
  COutputWidget(QWidget* parent, const char* name=0);
  /**destructor*/
  ~COutputWidget(){};

  void insertAtEnd(const QString& s);

protected:
  void mouseReleaseEvent(QMouseEvent* event);
  void keyPressEvent ( QKeyEvent* event);
  
signals:
  /** emited, if the mouse was clicked over the widget*/
  void clicked();
  void keyPressed(int key);
};

#endif

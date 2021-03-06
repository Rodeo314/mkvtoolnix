/*
   mkvinfo -- utility for gathering information about Matroska files

   Distributed under the GPL v2
   see the file COPYING for details
   or visit http://www.gnu.org/copyleft/gpl.html

   A Qt GUI for mkvinfo

   Written by Moritz Bunkus <moritz@bunkus.org>.
*/

#pragma once

#include "common/common_pch.h"

#include <QTreeWidget>

class rightclick_tree_widget: public QTreeWidget {
  Q_OBJECT;

public slots:

public:
  rightclick_tree_widget(QWidget *parent = nullptr);

protected:
  virtual void mousePressEvent(QMouseEvent *event);
};

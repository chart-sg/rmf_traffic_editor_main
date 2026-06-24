/*
 * Copyright (C) 2020-2021 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#ifndef ZONE_DIALOG_H
#define ZONE_DIALOG_H

#include <vector>

#include <QDialog>
#include <QObject>
#include <QRadioButton>

#include "zone.h"
#include "building.h"

class QLineEdit;
class QLabel;
class QTableWidget;
class QComboBox;
class QCheckBox;

class ZoneDialog : public QDialog
{
  Q_OBJECT

public:
  ZoneDialog(Zone& zone, Building& building);
  ~ZoneDialog();

private:
  Zone& _zone;
  Building& _building;

  Zone _zone_copy;
  std::vector<QString> _level_names;
  std::vector<QString> _vertex_names;
  std::vector<double> _vertex_x;
  std::vector<double> _vertex_y;
  std::vector<double> _level_elevations;

  QLineEdit* _name_line_edit;
  QComboBox* _internal_vertex_combo_box;
  QComboBox* _external_vertex_combo_box;
  QComboBox* _level_combo_box;
  QLineEdit* _x_line_edit;
  QLineEdit* _y_line_edit;
  QLineEdit* _yaw_line_edit;
  QLineEdit* _width_line_edit;
  QLineEdit* _depth_line_edit;
  QComboBox* _priority_box;
  QCheckBox* _exit_lanecheckbox;
  QCheckBox* _entry_lanecheckbox;
  QCheckBox* _vertexcheckbox;

  QTableWidget* _vertex_table;
  QTableWidget* _transition_lane_table;

  QGraphicsView* _zone_view;
  QGraphicsScene* _zone_scene;

  QPushButton* _ok_button, * _cancel_button;

  void update_vertex_table();
  void set_vertex_cell(const int row, const int col, const QString& text);
  void vertex_table_cell_changed(int row, int col);

  void update_transition_lane_table();
  void transition_lane_table_cell_changed(int row, int col);

  void update_zone_view();

  void update_internal_vertex_combobox();

  /// True if internal_vertex names a zone vertex and external_vertex a level vertex (not placeholders).
  bool transition_lane_vertices_valid(const ZoneTransitionLane& lane) const;
  /// True if some valid entry lane row differs from some valid exit lane row.
  bool has_valid_entry_and_exit_transition_lanes() const;
  /// True if zone name is unique
  bool is_zone_name_unique(const std::string& name) const;

  bool confirm_warning(const QString& text);

  bool is_vertex_name_unique(const std::string& name, int ignore_index) const;

private slots:
  void ok_button_clicked();
  void cancel_button_clicked();

signals:
  void redraw();
};

#endif

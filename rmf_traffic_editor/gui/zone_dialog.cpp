/*
 * Copyright (C) 2019-2020 Open Source Robotics Foundation
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

#include "zone_dialog.h"
#include <cfloat>
#include <QtWidgets>
using std::vector;


ZoneDialog::ZoneDialog(Zone& zone, Building& building)
: QDialog(),
  _zone(zone),
  _building(building)
{
  setWindowTitle("Zone Properties");
  for (const auto& level : building.levels)
  {
    _level_names.push_back(QString::fromStdString(level.name));
    _level_elevations.push_back(level.elevation);

    for (const auto& vertex : level.vertices)
    {
      if (vertex.name != "")
        _vertex_names.push_back(QString::fromStdString(vertex.name));
      _vertex_x.push_back(vertex.x);
      _vertex_y.push_back(vertex.y);
    }
  }

  _zone_copy = _zone;

  QHBoxLayout* bottom_buttons_hbox = new QHBoxLayout;
  _ok_button = new QPushButton("OK", this);  // first button = [enter] button
  bottom_buttons_hbox->addWidget(_ok_button);
  connect(
    _ok_button, &QAbstractButton::clicked,
    this, &ZoneDialog::ok_button_clicked);

  _cancel_button = new QPushButton("Cancel", this);
  bottom_buttons_hbox->addWidget(_cancel_button);
  connect(
    _cancel_button,
    &QAbstractButton::clicked,
    this,
    &ZoneDialog::cancel_button_clicked);

  QHBoxLayout* name_hbox = new QHBoxLayout;
  name_hbox->addWidget(new QLabel("Name:"));
  _name_line_edit =
    new QLineEdit(QString::fromStdString(_zone.name), this);
  connect(
    _name_line_edit,
    &QLineEdit::textEdited,
    [this](const QString& text)
    {
      _zone.name = text.toStdString();
      update_zone_view();
      emit redraw();
    });
  _name_line_edit->setToolTip("Name of the zone. (must be unique)");
  name_hbox->addWidget(_name_line_edit);

  QHBoxLayout* level_hbox = new QHBoxLayout;
  level_hbox->addWidget(new QLabel("Zone level:"));
  _level_combo_box = new QComboBox;
  for (const QString& level_name : _level_names)
  {
    _level_combo_box->addItem(level_name);
  }
  _level_combo_box->setCurrentText(
    QString::fromStdString(_zone.level));
  connect(
    _level_combo_box,
    &QComboBox::currentTextChanged,
    [this](const QString& text)
    {
      _zone.level = text.toStdString();
      for (std::size_t i = 0; i < _level_names.size(); i++)
      {
        if (text == _level_names[i])
        {
          _zone.zone_elevation = _level_elevations[i];
        }
      }
      emit redraw();

    });
  _level_combo_box->setToolTip("Level which the zone is in.");
  level_hbox->addWidget(_level_combo_box);

  QHBoxLayout* x_hbox = new QHBoxLayout;
  x_hbox->addWidget(new QLabel("X (pixel):"));
  _x_line_edit =
    new QLineEdit(QString::number(_zone.x), this);
  connect(
    _x_line_edit,
    &QLineEdit::textEdited,
    [this](const QString& text)
    {
      _zone.x = text.toDouble();
      emit redraw();
    });
  _x_line_edit->setToolTip(
    "This <X> refers to either the global <X> of the zone");
  x_hbox->addWidget(_x_line_edit);

  QHBoxLayout* y_hbox = new QHBoxLayout;
  y_hbox->addWidget(new QLabel("Y (pixel):"));
  _y_line_edit =
    new QLineEdit(QString::number(_zone.y), this);
  connect(
    _y_line_edit,
    &QLineEdit::textEdited,
    [this](const QString& text)
    {
      _zone.y = text.toDouble();
      emit redraw();
    });
  _y_line_edit->setToolTip(
    "This <Y> refers to either the global <Y> of the zone");
  y_hbox->addWidget(_y_line_edit);

  QHBoxLayout* yaw_hbox = new QHBoxLayout;
  yaw_hbox->addWidget(new QLabel("Yaw (rad):"));
  _yaw_line_edit =
    new QLineEdit(QString::number(_zone.yaw), this);
  connect(
    _yaw_line_edit,
    &QLineEdit::textEdited,
    [this](const QString& text)
    {
      _zone.yaw = text.toDouble();
      update_zone_view();
      emit redraw();
    });
  _yaw_line_edit->setToolTip(
    "This <Yaw> refers to either the global <Yaw> of the zone.\n The yaw is in radians.");
  yaw_hbox->addWidget(_yaw_line_edit);

  QHBoxLayout* width_hbox = new QHBoxLayout;
  width_hbox->addWidget(new QLabel("Zone width (m):"));
  _width_line_edit =
    new QLineEdit(QString::number(_zone.width), this);
  connect(
    _width_line_edit,
    &QLineEdit::textEdited,
    [this](const QString& text)
    {
      _zone.width = text.toDouble();
      update_zone_view();
      emit redraw();
    });
  _width_line_edit->setToolTip("Width of the zone.");
  width_hbox->addWidget(_width_line_edit);

  QHBoxLayout* depth_hbox = new QHBoxLayout;
  depth_hbox->addWidget(new QLabel("Zone depth (m):"));
  _depth_line_edit =
    new QLineEdit(QString::number(_zone.depth), this);
  connect(
    _depth_line_edit,
    &QLineEdit::textEdited,
    [this](const QString& text)
    {
      _zone.depth = text.toDouble();
      update_zone_view();
      emit redraw();
    });
  _depth_line_edit->setToolTip("Width of the zone.");
  depth_hbox->addWidget(_depth_line_edit);


  QHBoxLayout* entry_hbox = new QHBoxLayout;
  entry_hbox->addWidget(new QLabel("Entry point:"));
  _entry_combo_box = new QComboBox;
  update_entry_combobox();
  _entry_combo_box->setCurrentText(
    QString::fromStdString(_zone.entry_point));
  connect(
    _entry_combo_box,
    &QComboBox::currentTextChanged,
    [this](const QString& text)
    {
      _zone.entry_point = text.toStdString();
    });
  entry_hbox->addWidget(_entry_combo_box);

  QHBoxLayout* external_entry_hbox = new QHBoxLayout;
  external_entry_hbox->addWidget(new QLabel("External entry point:"));
  _external_entry_combo_box = new QComboBox;
  for (const QString& vertex_name : _vertex_names)
  {
    _external_entry_combo_box->addItem(vertex_name);
  }
  _external_entry_combo_box->setCurrentText(
    QString::fromStdString(_zone.external_entry_point));
  connect(
    _external_entry_combo_box,
    &QComboBox::currentTextChanged,
    [this](const QString& text)
    {
      _zone.external_entry_point = text.toStdString();
      for (std::size_t i = 0; i < _vertex_names.size(); i++)
      {
        if (_vertex_names[i].toStdString() == _zone.external_entry_point)
        {
          _zone.external_entry_point_x = _vertex_x[i];
          _zone.external_entry_point_y = _vertex_y[i];
        }
      }

    });
  _external_entry_combo_box->setToolTip("Level which the zone is in.");
  external_entry_hbox->addWidget(_external_entry_combo_box);

  QHBoxLayout* show_vertices_hbox = new QHBoxLayout;
  show_vertices_hbox->addWidget(new QLabel("Show vertices:"));
  _vertexcheckbox = new QCheckBox;
  _vertexcheckbox->setChecked(_zone.show_vertices);
  connect(
    _vertexcheckbox,
    &QAbstractButton::clicked,
    [this](bool box_checked)
    {
      _zone.show_vertices = box_checked;
      update_zone_view();
      emit redraw();
    });
  show_vertices_hbox->addWidget(_vertexcheckbox);

  _level_table = new QTableWidget;
  _level_table->setMinimumSize(200, 200);
  _level_table->verticalHeader()->setVisible(false);
  _level_table->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
  _level_table->setHorizontalHeaderItem(0, new QTableWidgetItem("Level name"));

  _vertex_table = new QTableWidget;
  _vertex_table->setMinimumSize(400, 200);
  _vertex_table->verticalHeader()->setVisible(false);
  _vertex_table->setColumnCount(6);
  _vertex_table->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

  _vertex_table->setHorizontalHeaderItem(0,
    new QTableWidgetItem("Vertex name"));
  _vertex_table->horizontalHeader()->setSectionResizeMode(
    0, QHeaderView::Stretch);

  _vertex_table->setHorizontalHeaderItem(1,
    new QTableWidgetItem("X (m)"));
  _vertex_table->horizontalHeader()->setSectionResizeMode(
    1, QHeaderView::ResizeToContents);

  _vertex_table->setHorizontalHeaderItem(2,
    new QTableWidgetItem("Y (m)"));
  _vertex_table->horizontalHeader()->setSectionResizeMode(
    2, QHeaderView::ResizeToContents);

  _vertex_table->setHorizontalHeaderItem(3,
    new QTableWidgetItem("Group"));
  _vertex_table->horizontalHeader()->setSectionResizeMode(
    3, QHeaderView::ResizeToContents);

  _vertex_table->setHorizontalHeaderItem(4,
    new QTableWidgetItem("Priority"));
  _vertex_table->horizontalHeader()->setSectionResizeMode(
    4, QHeaderView::ResizeToContents);

  _vertex_table->setHorizontalHeaderItem(5,
    new QTableWidgetItem("Add Vertex..."));
  _vertex_table->horizontalHeader()->setSectionResizeMode(
    5, QHeaderView::ResizeToContents);

  _transition_lane_table = new QTableWidget;
  _transition_lane_table->setMinimumSize(600, 200);
  _transition_lane_table->verticalHeader()->setVisible(false);
  _transition_lane_table->setColumnCount(5);

  _transition_lane_table->setHorizontalHeaderItem(0,
    new QTableWidgetItem("Internal vertex"));
  _transition_lane_table->horizontalHeader()->setSectionResizeMode(
    0, QHeaderView::Stretch);

  _transition_lane_table->setHorizontalHeaderItem(1,
    new QTableWidgetItem("External vertex"));
  _transition_lane_table->horizontalHeader()->setSectionResizeMode(
    1, QHeaderView::ResizeToContents);

  _transition_lane_table->setHorizontalHeaderItem(2,
    new QTableWidgetItem("is exit lane"));
  _transition_lane_table->horizontalHeader()->setSectionResizeMode(
    2, QHeaderView::ResizeToContents);

  _transition_lane_table->setHorizontalHeaderItem(3,
    new QTableWidgetItem("is entry lane"));
  _transition_lane_table->horizontalHeader()->setSectionResizeMode(
    3, QHeaderView::ResizeToContents);

  _transition_lane_table->setHorizontalHeaderItem(4,
    new QTableWidgetItem("Add lane..."));
  _transition_lane_table->horizontalHeader()->setSectionResizeMode(
    4, QHeaderView::ResizeToContents);

  QVBoxLayout* left_vbox = new QVBoxLayout;
  left_vbox->addLayout(name_hbox);
  left_vbox->addLayout(level_hbox);
  left_vbox->addLayout(width_hbox);
  left_vbox->addLayout(depth_hbox);
  left_vbox->addLayout(x_hbox);
  left_vbox->addLayout(y_hbox);
  left_vbox->addLayout(yaw_hbox);
  left_vbox->addWidget(_transition_lane_table);
  left_vbox->addLayout(show_vertices_hbox);

  QVBoxLayout* right_vbox = new QVBoxLayout;

  _zone_scene = new QGraphicsScene;

  _zone_view = new QGraphicsView;
  _zone_view->setScene(_zone_scene);
  _zone_view->setMinimumSize(400, 400);
  right_vbox->addWidget(_zone_view, 1);

  right_vbox->addWidget(_vertex_table);

  QHBoxLayout* top_hbox = new QHBoxLayout;
  top_hbox->addLayout(left_vbox);
  top_hbox->addLayout(right_vbox, 1);

  QVBoxLayout* top_vbox = new QVBoxLayout;
  top_vbox->addLayout(top_hbox);
  // todo: some sort of separator (?)
  top_vbox->addLayout(bottom_buttons_hbox);

  setLayout(top_vbox);

  _name_line_edit->setFocus(Qt::OtherFocusReason);

  update_vertex_table();
  update_transition_lane_table();
  update_level_table();

  connect(
    _vertex_table, &QTableWidget::cellChanged,
    this, &ZoneDialog::vertex_table_cell_changed);

  connect(
    _transition_lane_table, &QTableWidget::cellChanged,
    this, &ZoneDialog::transition_lane_table_cell_changed);

  update_zone_view();
  adjustSize();
}

// ======================================================================================================================
ZoneDialog::~ZoneDialog()
{
}

bool ZoneDialog::is_zone_name_unique(const std::string& name) const
{
  for (const auto& zone : _building.zones)
  {
    // skip comparing with itself (important when editing existing zone)
    if (&zone == &_zone)
      continue;

    if (zone.name == name)
      return false;
  }
  return true;
}

// ======================================================================================================================
void ZoneDialog::ok_button_clicked()
{
  if (_name_line_edit->text().isEmpty())
  {
    QMessageBox::critical(this, "Error", "Zone name is empty");
    return;
  }

  if (!has_valid_entry_and_exit_transition_lanes())
  {
    QMessageBox::critical(
      this,
      "Error",
      "Configure at least two valid rows, with one row marked as entry "
      "and another marked as exit. Two rows doesn't share the same "
      "external vertex");
    return;
  }

  _zone.name = _name_line_edit->text().toStdString();
  _zone.level = _level_combo_box->currentText().toStdString();
  _zone.entry_point = _entry_combo_box->currentText().toStdString();
  _zone.external_entry_point =
    _external_entry_combo_box->currentText().toStdString();

  for (std::size_t i = 0; i < _level_names.size(); i++)
  {
    if (_level_names[i].toStdString() == _zone.level)
    {
      _zone.zone_elevation = _level_elevations[i];
      break;
    }
  }

  _zone.x = _x_line_edit->text().toDouble();
  _zone.y = _y_line_edit->text().toDouble();
  _zone.yaw = _yaw_line_edit->text().toDouble();

  _zone.width = _width_line_edit->text().toDouble();
  _zone.depth = _depth_line_edit->text().toDouble();

  if (!is_zone_name_unique(_name_line_edit->text().toStdString()))
  {
    QMessageBox::critical(this, "Error", "Zone name must be unique");
    return;
  }

  update_zone_view();
  emit redraw();
  accept();
}

// ======================================================================================================================
void ZoneDialog::cancel_button_clicked()
{
  _zone = _zone_copy;
  update_zone_view();
  emit redraw();
  reject();
}

// ======================================================================================================================
void ZoneDialog::update_vertex_table()
{
  _vertex_table->setRowCount(1 + _zone.vertices.size());
  for (std::size_t i = 0; i < _zone.vertices.size(); i++)
  {
    const ZoneVertex& vertex = _zone.vertices[i];  // save some typing
    set_vertex_cell(i, 0, QString::fromStdString(vertex.name));

    // set the numeric fields
    set_vertex_cell(i, 1, QString::number(vertex.x));
    set_vertex_cell(i, 2, QString::number(vertex.y));

    // set the group name
    set_vertex_cell(i, 3, QString::fromStdString(vertex.group));

    _priority_box = new QComboBox;
    for (std::size_t j = 0; j < _zone.vertices.size(); j++)
    {
      _priority_box->addItem(QString::number(j+1));
    }
    _priority_box->setCurrentText(QString::number(vertex.priority));
    connect(
      _priority_box,
      &QComboBox::currentTextChanged,
      [this, i](const QString& text)
      {
        _zone.vertices[i].priority = text.toInt();
        update_vertex_table();
        update_zone_view();
        emit redraw();
      });
    _vertex_table->setCellWidget(i, 4, _priority_box);

    QPushButton* delete_button = new QPushButton("Delete...", this);
    _vertex_table->setCellWidget(i, 5, delete_button);

    connect(
      delete_button,
      &QAbstractButton::clicked,
      [this, i]()
      {
        _zone.vertices.erase(_zone.vertices.begin() + i);
        set_vertex_cell(_zone.vertices.size(), 0, nullptr);
        set_vertex_cell(_zone.vertices.size(), 1, nullptr);
        set_vertex_cell(_zone.vertices.size(), 2, nullptr);
        set_vertex_cell(_zone.vertices.size(), 3, nullptr);
        update_entry_combobox();
        update_vertex_table();
        update_zone_view();
      });
  }

  // we'll use the last row for the "Add" button
  const int last_row_idx = static_cast<int>(_zone.vertices.size());
  _vertex_table->setCellWidget(last_row_idx, 0, nullptr);
  _vertex_table->setCellWidget(last_row_idx, 1, nullptr);
  _vertex_table->setCellWidget(last_row_idx, 2, nullptr);
  _vertex_table->setCellWidget(last_row_idx, 3, nullptr);
  _vertex_table->setCellWidget(last_row_idx, 4, nullptr);
  QPushButton* add_button = new QPushButton("Add...", this);
  _vertex_table->setCellWidget(last_row_idx, 5, add_button);
  connect(
    add_button,
    &QAbstractButton::clicked,
    [this]()
    {
      if (!is_vertex_name_unique("name", -1))
      {
        QMessageBox::warning(this, "Duplicate name", 
          "A vertex with the name 'name' already exists. Please change the name of existing vertex first.");
        return;
      }
      ZoneVertex vertex;
      vertex.name = "name";
      vertex.priority = _zone.vertices.size()+1;
      _zone.vertices.push_back(vertex);
      update_entry_combobox();
      update_vertex_table();
      update_zone_view();
    });
}

// ======================================================================================================================
void ZoneDialog::update_transition_lane_table()
{
  _transition_lane_table->setRowCount(1 + _zone.transition_lanes.size());

  for (std::size_t i = 0; i < _zone.transition_lanes.size(); i++)
  {
    const ZoneTransitionLane& lane = _zone.transition_lanes[i];  // save some typing

    // set_transition_lane_cell(i, 0, QString::fromStdString(lane.internal_vertex));
    // set_transition_lane_cell(i, 1, QString::fromStdString(lane.external_vertex));

    _internal_vertex_combo_box = new QComboBox;
    update_internal_vertex_combobox();
    _internal_vertex_combo_box->setCurrentText(
      QString::fromStdString(lane.internal_vertex));
    connect(
      _internal_vertex_combo_box,
      &QComboBox::currentTextChanged,
      [this, i](const QString& text)
      {
        _zone.transition_lanes[i].internal_vertex = text.toStdString();
      });
    _transition_lane_table->setCellWidget(i, 0, _internal_vertex_combo_box);


    _external_vertex_combo_box = new QComboBox;
    // update_external_vertex_combobox();
    _external_vertex_combo_box->addItem("Select vertex");
    for (const QString& vertex_name : _vertex_names)
    {
      _external_vertex_combo_box->addItem(vertex_name);
    }
    _external_vertex_combo_box->setCurrentText(
      QString::fromStdString(lane.external_vertex));
    connect(
      _external_vertex_combo_box,
      &QComboBox::currentTextChanged,
      [this, i](const QString& text)
      {
        _zone.transition_lanes[i].external_vertex = text.toStdString();
      });
    _transition_lane_table->setCellWidget(i, 1, _external_vertex_combo_box);

    _exit_lanecheckbox = new QCheckBox;
    _exit_lanecheckbox->setChecked(lane.is_exit_lane);
    connect(
      _exit_lanecheckbox,
      &QAbstractButton::clicked,
      [this, i](bool box_checked)
      {
        _zone.transition_lanes[i].is_exit_lane = box_checked;
      });
    _transition_lane_table->setCellWidget(i, 2, _exit_lanecheckbox);


    QCheckBox* entry_lanecheckbox = new QCheckBox;
    entry_lanecheckbox->setChecked(lane.is_entry_lane);
    connect(
      entry_lanecheckbox,
      &QAbstractButton::clicked,
      [this, i](bool box_checked)
      {
        _zone.transition_lanes[i].is_entry_lane = box_checked;
      });
    _transition_lane_table->setCellWidget(i, 3, entry_lanecheckbox);

    QPushButton* delete_button = new QPushButton("Delete...", this);
    _transition_lane_table->setCellWidget(i, 4, delete_button);

    connect(
      delete_button,
      &QAbstractButton::clicked,
      [this, i]()
      {
        if (_zone.transition_lanes.size() <= 2)
        {
          QMessageBox::warning(
            this,
            "Cannot delete",
            "At least two transition lanes are required "
            "(internal ↔ external vertex each).");
          return;
        }
        _zone.transition_lanes.erase(_zone.transition_lanes.begin() + i);
        set_transition_lane_cell(_zone.transition_lanes.size(), 0, nullptr);
        set_transition_lane_cell(_zone.transition_lanes.size(), 1, nullptr);
        _transition_lane_table->setCellWidget(_zone.transition_lanes.size(), 2,
        nullptr);
        _transition_lane_table->setCellWidget(_zone.transition_lanes.size(), 3,
        nullptr);
        update_transition_lane_table();
        update_zone_view();
      });
  }

  // we'll use the last row for the "Add" button
  const int last_row_idx = static_cast<int>(_zone.transition_lanes.size());
  _transition_lane_table->setCellWidget(last_row_idx, 0, nullptr);
  _transition_lane_table->setCellWidget(last_row_idx, 1, nullptr);
  _transition_lane_table->setCellWidget(last_row_idx, 2, nullptr);
  _transition_lane_table->setCellWidget(last_row_idx, 3, nullptr);
  QPushButton* add_button = new QPushButton("Add...", this);
  _transition_lane_table->setCellWidget(last_row_idx, 4, add_button);
  connect(
    add_button,
    &QAbstractButton::clicked,
    [this]()
    {
      ZoneTransitionLane new_transition_lane;
      _zone.transition_lanes.push_back(new_transition_lane);
      update_transition_lane_table();
      update_zone_view();
    });
}
// ======================================================================================================================
void ZoneDialog::update_entry_combobox()
{
  _entry_combo_box->clear();
  for (const auto& vertex : _zone.vertices)
  {
    _entry_combo_box->addItem(QString::fromStdString(vertex.name));
  }
}
// ======================================================================================================================
void ZoneDialog::update_internal_vertex_combobox()
{
  _internal_vertex_combo_box->clear();
  _internal_vertex_combo_box->addItem(QString::fromStdString(
      "<select vertex>"));
  for (const auto& vertex : _zone.vertices)
  {
    _internal_vertex_combo_box->addItem(QString::fromStdString(vertex.name));
  }
}

bool ZoneDialog::transition_lane_vertices_valid(const ZoneTransitionLane& lane)
const
{
  static const char* const k_internal_placeholder = "<select vertex>";
  static const char* const k_external_placeholder = "Select vertex";

  if (lane.internal_vertex.empty() ||
    lane.internal_vertex == k_internal_placeholder)
  {
    return false;
  }
  if (lane.external_vertex.empty() ||
    lane.external_vertex == k_external_placeholder)
  {
    return false;
  }

  bool internal_ok = false;
  for (const auto& vertex : _zone.vertices)
  {
    if (vertex.name == lane.internal_vertex)
    {
      internal_ok = true;
      break;
    }
  }
  if (!internal_ok)
  {
    return false;
  }

  const QString external_q = QString::fromStdString(lane.external_vertex);
  for (const QString& name : _vertex_names)
  {
    if (name == external_q)
    {
      return true;
    }
  }
  return false;
}

bool ZoneDialog::has_valid_entry_and_exit_transition_lanes() const
{
  // Check that each internal vertex has at least one entry lane and one exit lane
  for (const auto& vertex : _zone.vertices)
  {
    int entry_count = 0;
    int exit_count = 0;
    
    for (const auto& lane : _zone.transition_lanes)
    {
      if (!transition_lane_vertices_valid(lane))
        continue;
      
      if (lane.internal_vertex == vertex.name)
      {
        if (lane.is_entry_lane)
          entry_count++;
        if (lane.is_exit_lane)
          exit_count++;
      }
    }
    
    if (entry_count < 1 || exit_count < 1)
      return false;
  }

  // Also check that there's at least one entry lane and one exit lane (can be the same lane)
  // with different external vertices
  for (const auto& entry_lane : _zone.transition_lanes)
  {
    if (!entry_lane.is_entry_lane)
      continue;

    for (const auto& exit_lane : _zone.transition_lanes)
    {
      if (!exit_lane.is_exit_lane)
        continue;

      if (entry_lane.external_vertex != exit_lane.external_vertex)
        return true;
    }
  }

  return false;
}

// ======================================================================================================================
void ZoneDialog::set_vertex_cell(const int row, const int col,
  const QString& text)
{
  _vertex_table->setItem(row, col, new QTableWidgetItem(text));
}
// ======================================================================================================================
void ZoneDialog::set_transition_lane_cell(const int row, const int col,
  const QString& text)
{
  _transition_lane_table->setItem(row, col, new QTableWidgetItem(text));
}

// ======================================================================================================================
void ZoneDialog::update_level_table()
{
  _level_table->setColumnCount(1 + static_cast<int>(_zone.vertices.size()));
  _level_table->setHorizontalHeaderItem(0, new QTableWidgetItem("Level"));
  for (std::size_t vertex_idx = 0; vertex_idx < _zone.vertices.size();
    vertex_idx++)
  {
    _level_table->setHorizontalHeaderItem(
      vertex_idx + 1,
      new QTableWidgetItem(
        QString::fromStdString(_zone.vertices[vertex_idx].name)));
  }
}
// ======================================================================================================================
bool ZoneDialog::is_vertex_name_unique(const std::string& name, int ignore_index) const
{
  for (std::size_t i = 0; i < _zone.vertices.size(); ++i)
  {
    if (static_cast<int>(i) == ignore_index)
      continue;  // skip itself when editing

    if (_zone.vertices[i].name == name)
      return false;
  }
  return true;
}
// ======================================================================================================================
void ZoneDialog::vertex_table_cell_changed(int row, int col)
{
  // printf("vertex_table_cell_changed(%d, %d)\n", row, col);
  if (row >= static_cast<int>(_zone.vertices.size()))
  {
    printf("invalid zone row: %d\n", row);
    return;  // let's not crash
  }

  // If a vertex name was changed, we need to update the options shown in all
  // the level_table combo boxes
  if (col == 0)  // name
  {
    std::string new_name =
      _vertex_table->item(row, col)->text().toStdString();

    if (new_name.empty())
    {
      QMessageBox::warning(this, "Invalid name", "Vertex name cannot be empty");
      return;
    }

    if (!is_vertex_name_unique(new_name, row))
    {
      QMessageBox::warning(this, "Duplicate name", "Vertex name must be unique");

      // revert to previous value
      _vertex_table->item(row, col)->setText(
        QString::fromStdString(_zone.vertices[row].name));
      return;
    }

    _zone.vertices[row].name = new_name;

    update_entry_combobox();
    update_level_table();
  }
  else if (col == 1) // x
    _zone.vertices[row].x = _vertex_table->item(row, col)->text().toDouble();
  else if (col == 2) // y
    _zone.vertices[row].y = _vertex_table->item(row, col)->text().toDouble();
  else if (col == 3)
    _zone.vertices[row].group =
      _vertex_table->item(row, col)->text().toStdString();
  else if (col == 4)
    _zone.vertices[row].priority =
      _vertex_table->item(row, col)->text().toInt();

  update_transition_lane_table();
  update_zone_view();
  emit redraw();
}
// ======================================================================================================================
void ZoneDialog::transition_lane_table_cell_changed(int row, int col)
{
  // printf("vertex_table_cell_changed(%d, %d)\n", row, col);
  if (row >= static_cast<int>(_zone.transition_lanes.size()))
  {
    printf("invalid zone row: %d\n", row);
    return;  // let's not crash
  }

  // If a vertex name was changed, we need to update the options shown in all
  // the level_table combo boxes
  if (col == 0) // name
    _zone.transition_lanes[row].internal_vertex = _transition_lane_table->item(
      row, col)->text().toStdString();
  else if (col == 1) // x
    _zone.transition_lanes[row].external_vertex = _transition_lane_table->item(
      row, col)->text().toStdString();
  else if (col == 2) // y
    _zone.transition_lanes[row].is_exit_lane = _transition_lane_table->item(
      row, col)->text().toInt();
  else if (col == 3)
    _zone.transition_lanes[row].is_entry_lane = _transition_lane_table->item(
      row, col)->text().toInt();

  update_internal_vertex_combobox();
  update_level_table();
  update_zone_view();
  emit redraw();
}
// ======================================================================================================================
void ZoneDialog::update_zone_view()
{
  _zone_scene->clear();
  _zone.draw(_zone_scene, 0.01, std::string(), _zone.zone_elevation, false);
}

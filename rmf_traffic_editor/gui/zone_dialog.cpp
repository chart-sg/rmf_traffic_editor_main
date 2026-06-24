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
    "This <X> refers to the global <X> of the zone");
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
    "This <Y> refers to  the global <Y> of the zone");
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
    "This <Yaw> refers to the global <Yaw> of the zone.\n The yaw is in radians.");
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
  _depth_line_edit->setToolTip("Depth of the zone.");
  depth_hbox->addWidget(_depth_line_edit);

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

  _vertex_table = new QTableWidget;
  _vertex_table->setMinimumSize(400, 200);
  _vertex_table->verticalHeader()->setVisible(false);
  _vertex_table->setColumnCount(6);
  _vertex_table->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

  _vertex_table->setHorizontalHeaderItem(0,
    new QTableWidgetItem("Add Internal Vertex..."));
  _vertex_table->horizontalHeader()->setSectionResizeMode(
    0, QHeaderView::ResizeToContents);

  _vertex_table->setHorizontalHeaderItem(1,
    new QTableWidgetItem("Vertex name"));
  _vertex_table->horizontalHeader()->setSectionResizeMode(
    1, QHeaderView::Stretch);

  _vertex_table->setHorizontalHeaderItem(2,
    new QTableWidgetItem("X (m)"));
  _vertex_table->horizontalHeader()->setSectionResizeMode(
    2, QHeaderView::ResizeToContents);

  _vertex_table->setHorizontalHeaderItem(3,
    new QTableWidgetItem("Y (m)"));
  _vertex_table->horizontalHeader()->setSectionResizeMode(
    3, QHeaderView::ResizeToContents);

  _vertex_table->setHorizontalHeaderItem(4,
    new QTableWidgetItem("Group"));
  _vertex_table->horizontalHeader()->setSectionResizeMode(
    4, QHeaderView::ResizeToContents);

  _vertex_table->setHorizontalHeaderItem(5,
    new QTableWidgetItem("Priority"));
  _vertex_table->horizontalHeader()->setSectionResizeMode(
    5, QHeaderView::ResizeToContents);

  _transition_lane_table = new QTableWidget;
  _transition_lane_table->setMinimumSize(600, 200);
  _transition_lane_table->verticalHeader()->setVisible(false);
  _transition_lane_table->setColumnCount(5);

  _transition_lane_table->setHorizontalHeaderItem(0,
    new QTableWidgetItem("Add lane..."));
  _transition_lane_table->horizontalHeader()->setSectionResizeMode(
    0, QHeaderView::ResizeToContents);

  _transition_lane_table->setHorizontalHeaderItem(1,
    new QTableWidgetItem("Internal vertex"));
  _transition_lane_table->horizontalHeader()->setSectionResizeMode(
    1, QHeaderView::Stretch);

  _transition_lane_table->setHorizontalHeaderItem(2,
    new QTableWidgetItem("External vertex"));
  _transition_lane_table->horizontalHeader()->setSectionResizeMode(
    2, QHeaderView::ResizeToContents);

  _transition_lane_table->setHorizontalHeaderItem(3,
    new QTableWidgetItem("is exit lane"));
  _transition_lane_table->horizontalHeader()->setSectionResizeMode(
    3, QHeaderView::ResizeToContents);

  _transition_lane_table->setHorizontalHeaderItem(4,
    new QTableWidgetItem("is entry lane"));
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
  
  QVBoxLayout* right_vbox = new QVBoxLayout;
  _zone_scene = new QGraphicsScene;
  _zone_view = new QGraphicsView;
  _zone_view->setScene(_zone_scene);
  _zone_view->setMinimumSize(400, 400);
  right_vbox->addWidget(_zone_view, 1);
  right_vbox->addWidget(_vertex_table);
  right_vbox->addLayout(show_vertices_hbox);

  QHBoxLayout* top_hbox = new QHBoxLayout;
  top_hbox->addLayout(left_vbox);
  top_hbox->addLayout(right_vbox, 1);

  QVBoxLayout* top_vbox = new QVBoxLayout;
  top_vbox->addLayout(top_hbox);
  // todo: some sort of separator (?)
  top_vbox->addLayout(bottom_buttons_hbox);

  setLayout(top_vbox);

  update_vertex_table();
  update_transition_lane_table();

  connect(
    _vertex_table, &QTableWidget::cellChanged,
    this, &ZoneDialog::vertex_table_cell_changed);

  connect(
    _transition_lane_table, &QTableWidget::cellChanged,
    this, &ZoneDialog::transition_lane_table_cell_changed);

  update_zone_view();
  adjustSize();
  _ok_button->setFocus(Qt::OtherFocusReason);
  _ok_button->setDefault(true);
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

  if (!is_zone_name_unique(_name_line_edit->text().toStdString()))
  {
    QMessageBox::critical(this, "Error", "Zone name must be unique");
    return;
  }
  
  if (!has_valid_entry_and_exit_transition_lanes())
  {
    if (!confirm_warning(
      "Current lanes configuration might cause issues when robots "
      "enter/exit the zone"))
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
  _vertex_table->blockSignals(true);
  _vertex_table->setRowCount(1 + _zone.vertices.size());
  for (std::size_t i = 0; i < _zone.vertices.size(); i++)
  {
    const ZoneVertex& vertex = _zone.vertices[i];  // save some typing

    // clear any lingering placeholder items (e.g. when a row was previously the Add row)
    _vertex_table->setItem(i, 0, new QTableWidgetItem());
    _vertex_table->setItem(i, 5, new QTableWidgetItem());

    QPushButton* delete_button = new QPushButton("Delete...", this);
    _vertex_table->setCellWidget(i, 0, delete_button);

    set_vertex_cell(i, 1, QString::fromStdString(vertex.name));

    // set the numeric fields
    set_vertex_cell(i, 2, QString::number(vertex.x));
    set_vertex_cell(i, 3, QString::number(vertex.y));

    // set the group name
    set_vertex_cell(i, 4, QString::fromStdString(vertex.group));

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
    _vertex_table->setCellWidget(i, 5, _priority_box);

    connect(
      delete_button,
      &QAbstractButton::clicked,
      [this, i]()
      {
        _zone.vertices.erase(_zone.vertices.begin() + i);
        update_vertex_table();
        update_transition_lane_table();
        update_zone_view();
      });
  }

  // we'll use the last row for the "Add" button
  const int last_row_idx = static_cast<int>(_zone.vertices.size());
  QPushButton* add_button = new QPushButton("Add...", this);
  _vertex_table->setCellWidget(last_row_idx, 0, add_button);
  for (int col = 1; col <= 5; col++)
  {
    _vertex_table->setCellWidget(last_row_idx, col, nullptr);
    auto* placeholder = new QTableWidgetItem();
    placeholder->setFlags(Qt::NoItemFlags);
    placeholder->setBackground(QColor(180, 180, 180));
    _vertex_table->setItem(last_row_idx, col, placeholder);
  }
  connect(
    add_button,
    &QAbstractButton::clicked,
    [this]()
    {
      if (!is_vertex_name_unique("name", -1))
      {
        QMessageBox::warning(this, "Duplicate name",
        "A vertex with the name 'name' already exists."
        " Please change the name of existing vertex first.");
        return;
      }
      ZoneVertex vertex;
      vertex.name = "name";
      vertex.priority = _zone.vertices.size()+1;
      _zone.vertices.push_back(vertex);
      update_vertex_table();
      update_transition_lane_table();
      update_zone_view();
    });
  _vertex_table->blockSignals(false);
}

// ======================================================================================================================
void ZoneDialog::update_transition_lane_table()
{
  _transition_lane_table->blockSignals(true);
  _transition_lane_table->setRowCount(1 + _zone.transition_lanes.size());

  for (std::size_t i = 0; i < _zone.transition_lanes.size(); i++)
  {
    const ZoneTransitionLane& lane = _zone.transition_lanes[i];  // save some typing

    // clear any lingering placeholder items (e.g. when a row was previously the Add row)
    for (int col = 0; col <= 4; col++)
      _transition_lane_table->setItem(i, col, new QTableWidgetItem());

    QPushButton* delete_button = new QPushButton("Delete...", this);
    _transition_lane_table->setCellWidget(i, 0, delete_button);

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
    _transition_lane_table->setCellWidget(i, 1, _internal_vertex_combo_box);

    _external_vertex_combo_box = new QComboBox;
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
    _transition_lane_table->setCellWidget(i, 2, _external_vertex_combo_box);

    _exit_lanecheckbox = new QCheckBox;
    _exit_lanecheckbox->setChecked(lane.is_exit_lane);
    connect(
      _exit_lanecheckbox,
      &QAbstractButton::clicked,
      [this, i](bool box_checked)
      {
        _zone.transition_lanes[i].is_exit_lane = box_checked;
      });
    _transition_lane_table->setCellWidget(i, 3, _exit_lanecheckbox);

    _entry_lanecheckbox = new QCheckBox;
    _entry_lanecheckbox->setChecked(lane.is_entry_lane);
    connect(
      _entry_lanecheckbox,
      &QAbstractButton::clicked,
      [this, i](bool box_checked)
      {
        _zone.transition_lanes[i].is_entry_lane = box_checked;
      });
    _transition_lane_table->setCellWidget(i, 4, _entry_lanecheckbox);

    connect(
      delete_button,
      &QAbstractButton::clicked,
      [this, i]()
      {
        _zone.transition_lanes.erase(_zone.transition_lanes.begin() + i);
        update_transition_lane_table();
        update_zone_view();
      });
  }

  // we'll use the last row for the "Add" button
  const int last_row_idx = static_cast<int>(_zone.transition_lanes.size());
  QPushButton* add_button = new QPushButton("Add...", this);
  _transition_lane_table->setCellWidget(last_row_idx, 0, add_button);
  for (int col = 1; col <= 4; col++)
  {
    _transition_lane_table->setCellWidget(last_row_idx, col, nullptr);
    auto* placeholder = new QTableWidgetItem();
    placeholder->setFlags(Qt::NoItemFlags);
    placeholder->setBackground(QColor(180, 180, 180));
    _transition_lane_table->setItem(last_row_idx, col, placeholder);
  }
  connect(
    add_button,
    &QAbstractButton::clicked,
    [this]()
    {
      if (_zone.vertices.size() < 1)
      {
        QMessageBox::warning(
          this,
          "Cannot add lane",
          "At least one internal vertex is required ");
        return;
      }
      ZoneTransitionLane new_transition_lane;
      _zone.transition_lanes.push_back(new_transition_lane);
      update_transition_lane_table();
      update_zone_view();
    });
  _transition_lane_table->blockSignals(false);
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
  for (const auto& vertex : _zone.vertices)
  {
    bool has_valid_pair = false;

    for (const auto& entry_lane : _zone.transition_lanes)
    {
      if (!transition_lane_vertices_valid(entry_lane))
        continue;
      if (!entry_lane.is_entry_lane)
        continue;
      if (entry_lane.internal_vertex != vertex.name)
        continue;

      for (const auto& exit_lane : _zone.transition_lanes)
      {
        if (!transition_lane_vertices_valid(exit_lane))
          continue;
        if (!exit_lane.is_exit_lane)
          continue;
        if (exit_lane.internal_vertex != vertex.name)
          continue;

        if (entry_lane.external_vertex != exit_lane.external_vertex)
        {
          has_valid_pair = true;
          break;
        }
      }

      if (has_valid_pair)
        break;
    }

    if (!has_valid_pair)
      return false;
  }

  return true;
}

// ======================================================================================================================
bool ZoneDialog::confirm_warning(const QString& text)
{
  QDialog dialog(this);
  dialog.setWindowTitle("Warning");

  QHBoxLayout* msg_hbox = new QHBoxLayout;
  QLabel* icon_label = new QLabel;
  icon_label->setPixmap(
    dialog.style()->standardIcon(QStyle::SP_MessageBoxWarning).pixmap(48, 48));
  msg_hbox->addWidget(icon_label);
  msg_hbox->addWidget(new QLabel(text), 1);

  QPushButton* ok_button = new QPushButton("OK");
  QPushButton* cancel_button = new QPushButton("Cancel");
  QHBoxLayout* button_hbox = new QHBoxLayout;
  button_hbox->addStretch(1);
  button_hbox->addWidget(ok_button);
  button_hbox->addWidget(cancel_button);

  QVBoxLayout* vbox = new QVBoxLayout;
  vbox->addLayout(msg_hbox);
  vbox->addLayout(button_hbox);
  dialog.setLayout(vbox);

  connect(ok_button, &QAbstractButton::clicked, &dialog, &QDialog::accept);
  connect(cancel_button, &QAbstractButton::clicked, &dialog, &QDialog::reject);

  return dialog.exec() == QDialog::Accepted;
}

// ======================================================================================================================
void ZoneDialog::set_vertex_cell(const int row, const int col,
  const QString& text)
{
  _vertex_table->setItem(row, col, new QTableWidgetItem(text));
}

// ======================================================================================================================
bool ZoneDialog::is_vertex_name_unique(const std::string& name,
  int ignore_index) const
{
  for (std::size_t i = 0; i < _zone.vertices.size(); ++i)
  {
    // skip itself when editing
    if (static_cast<int>(i) == ignore_index)
      continue;

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
  if (col == 1)  // name
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
      QMessageBox::warning(this, "Duplicate name",
        "Vertex name must be unique");

      // revert to previous value
      _vertex_table->item(row, col)->setText(
        QString::fromStdString(_zone.vertices[row].name));
      return;
    }

    _zone.vertices[row].name = new_name;
  }
  else if (col == 2) // x
    _zone.vertices[row].x = _vertex_table->item(row, col)->text().toDouble();
  else if (col == 3) // y
    _zone.vertices[row].y = _vertex_table->item(row, col)->text().toDouble();
  else if (col == 4) // group
    _zone.vertices[row].group =
      _vertex_table->item(row, col)->text().toStdString();
  else if (col == 5) // priority
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
  if (col == 1) // internal vertex
    _zone.transition_lanes[row].internal_vertex = _transition_lane_table->item(
      row, col)->text().toStdString();
  else if (col == 2) // external vertex
    _zone.transition_lanes[row].external_vertex = _transition_lane_table->item(
      row, col)->text().toStdString();
  else if (col == 3) // is exit lane
    _zone.transition_lanes[row].is_exit_lane = _transition_lane_table->item(
      row, col)->text().toInt();
  else if (col == 4) // is entry lane
    _zone.transition_lanes[row].is_entry_lane = _transition_lane_table->item(
      row, col)->text().toInt();

  update_internal_vertex_combobox();
  update_zone_view();
  emit redraw();
}
// ======================================================================================================================
void ZoneDialog::update_zone_view()
{
  _zone_scene->clear();
  _zone.draw(_zone_scene, 0.01, std::string(), _zone.zone_elevation, false);
}

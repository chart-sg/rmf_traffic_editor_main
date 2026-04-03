/*
 * Copyright (C) 2019-2021 Open Source Robotics Foundation
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

#include <algorithm>
#include <cmath>
#include <iostream>

#include <QGraphicsScene>
#include <QGraphicsSimpleTextItem>

#include "zone.h"
using std::string;

YAML::Node ZoneVertex::to_yaml() const
{
  YAML::Node n;
  n["x"] = std::round(x * 1000.0) / 1000.0;
  n["y"] = std::round(y * 1000.0) / 1000.0;
  n["priority"] = priority;
  n["group"] = group;

  return n;
}

void ZoneVertex::from_yaml(const std::string& _name, const YAML::Node& data)
{
  if (!data.IsMap())
    throw std::runtime_error("ZoneVertex::from_yaml() expected a map");
  name = _name;
  x = data["x"].as<double>();
  y = data["y"].as<double>();
  priority = data["priority"].as<int>();
  group = data["group"].as<std::string>();
}

YAML::Node ZoneTransitionLane::to_yaml() const
{
  YAML::Node n;
  n["internal_vertex"] = internal_vertex;
  n["external_vertex"] = external_vertex;
  n["is_exit_lane"] = is_exit_lane;
  n["is_entry_lane"] = is_entry_lane;

  return n;
}

void ZoneTransitionLane::from_yaml(const YAML::Node& data)
{
  is_exit_lane = data["is_exit_lane"].as<bool>();
  is_entry_lane = data["is_entry_lane"].as<bool>();
  external_vertex = data["external_vertex"].as<std::string>();
  internal_vertex = data["internal_vertex"].as<std::string>();
}

Zone::Zone()
{
}

void Zone::from_yaml(
  const std::string& _name,
  const YAML::Node& data,
  const std::vector<Level>& levels)
{
  if (!data.IsMap())
    throw std::runtime_error("Zone::from_yaml() expected a map");
  name = _name;
  x = data["x"].as<double>();
  y = data["y"].as<double>();
  yaw = data["yaw"].as<double>();
  level = data["level"].as<string>();

  switch (data["type"].as<int>())
  {
    case 1:
      type = "Parking zone";
      break;
    default:
      type = "Default";
  }

  for (const auto& map_level : levels)
  {
    if (map_level.name == level)
    {
      zone_elevation = map_level.elevation;
      break;
    }
  }

  width = data["width"].as<double>();
  depth = data["depth"].as<double>();

  if (data["vertices"] && data["vertices"].IsMap())
  {
    const YAML::Node& yd = data["vertices"];
    for (YAML::const_iterator it = yd.begin(); it != yd.end(); ++it)
    {
      ZoneVertex vertex;
      std::cout << "Loading vertex " << it->first.as<string>() << " for zone " << name << std::endl;
      vertex.from_yaml(it->first.as<string>(), it->second);
      vertices.push_back(vertex);
    }
  }

  if (data["transition_lanes"] && data["transition_lanes"].IsSequence())
  {
    for (const auto& lane : data["transition_lanes"])
    {
      ZoneTransitionLane transition_lane;
      transition_lane.from_yaml(lane);
      transition_lanes.push_back(transition_lane);
    }
  }

}

YAML::Node Zone::to_yaml() const
{
  YAML::Node n;
  n["x"] = std::round(x * 1000.0) / 1000.0;
  n["y"] = std::round(y * 1000.0) / 1000.0;
  // let's give yaw another decimal place because, I don't know, reasons (?)
  n["yaw"] = std::round(yaw * 10000.0) / 10000.0;
  n["level"] = level;
  n["width"] = std::round(width * 1000.0) / 1000.0;
  n["depth"] = std::round(depth * 1000.0) / 1000.0;

  if (type == "Parking zone")
    n["type"] = 1;
  else
    n["type"] = 0;

  n["vertices"] = YAML::Node(YAML::NodeType::Map);
  for (const auto& vertex : vertices)
    n["vertices"][vertex.name] = vertex.to_yaml();
  n["transition_lanes"] = YAML::Node(YAML::NodeType::Sequence);
  for (const auto& transition_lane : transition_lanes)
    n["transition_lanes"].push_back(transition_lane.to_yaml());
  return n;
}

void Zone::draw(
  QGraphicsScene* scene,
  const double meters_per_pixel,
  const string& level_name,
  const double elevation,
  const bool apply_transformation,
  const double scale,
  const double translate_x,
  const double translate_y) const
{
  if (!show_zone)
    return;

  if (elevation != zone_elevation)
    return;
  const double cabin_w = width / meters_per_pixel;
  const double cabin_d = depth / meters_per_pixel;
  QPen cabin_pen(Qt::black);
  cabin_pen.setWidth(0.05 / meters_per_pixel);

  QGraphicsRectItem* cabin_rect = new QGraphicsRectItem(
    -cabin_w / 2.0,
    -cabin_d / 2.0,
    cabin_w,
    cabin_d);
  cabin_rect->setPen(cabin_pen);
  cabin_rect->setBrush(QBrush(QColor::fromRgbF(0.3, 0.3, 1.0, 0.2)));
  scene->addItem(cabin_rect);

  QList<QGraphicsItem*> items;
  items.append(cabin_rect);

  if (!name.empty())
  {
    QFont font("Helvetica");
    font.setPointSize(0.2 / meters_per_pixel);
    QGraphicsSimpleTextItem* text_item = scene->addSimpleText(
      QString::fromStdString(name), font);
    text_item->setBrush(QColor(255, 0, 0, 255));
    text_item->setPos(-cabin_w / 3.0, 0.0);
    items.append(text_item);
  }

  for (const auto& vertex : vertices)
  {
    if (show_vertices)
    {
      QPen vertex_pen(Qt::black);
      double radius = 0.1 / meters_per_pixel;
      vertex_pen.setWidthF(radius / 2.0);

      QColor nonselected_color(QColor::fromRgbF(0.0, 0.5, 0.0));
      nonselected_color.setAlphaF(0.5);

      const QBrush vertex_brush = QBrush(nonselected_color);

      QGraphicsEllipseItem* ellipse_item = scene->addEllipse(
        vertex.x /meters_per_pixel - radius,
        vertex.y /meters_per_pixel - radius,
        2 * radius,
        2 * radius,
        vertex_pen,
        vertex_brush);
      ellipse_item->setZValue(20.0);  // above all lane/wall edges

      // add some icons depending on the superpowers of this vertex
      QPen annotation_pen(Qt::black);
      annotation_pen.setWidthF(radius / 4.0);
      const double icon_ring_radius = radius * 2.5;
      const double icon_scale = 2.0 * radius / 128.0;

      items.append(ellipse_item);

      QFont font("Helvetica");
      font.setPointSize(0.15 / meters_per_pixel);
      QGraphicsSimpleTextItem* priority_text_item = scene->addSimpleText(
        QString::number(vertex.priority), font);
      priority_text_item->setBrush(QColor(255, 0, 0, 255));
      priority_text_item->setPos(vertex.x /meters_per_pixel - radius,
        vertex.y /meters_per_pixel - radius);
      priority_text_item->setZValue(30.0);
      items.append(priority_text_item);

      font.setPointSize(0.1 / meters_per_pixel);
      QGraphicsSimpleTextItem* name_text_item = scene->addSimpleText(
        QString::fromStdString(vertex.name), font);
      name_text_item->setBrush(QColor(255, 0, 0, 255));
      name_text_item->setPos(vertex.x /meters_per_pixel + radius,
        vertex.y /meters_per_pixel + radius);
      name_text_item->setZValue(30.0);
      items.append(name_text_item);
    }
  }

  QGraphicsItemGroup* group = scene->createItemGroup(items);
  group->setZValue(95.0);
  if (apply_transformation)
  {
    group->setPos(x * scale + translate_x, y * scale + translate_y);
    group->setRotation(-180.0 / 3.1415926 * yaw);
  }
}

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

#ifndef ZONE_H
#define ZONE_H

class QGraphicsScene;
class QGraphicsView;

#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>
#include <cfloat>
#include "level.h"

class ZoneVertex
{
public:

  std::string name;

  double x = 0.0;  // x location of vertex in meters
  double y = 0.0;  // y location of vertex in meters
  std::size_t priority = 0; // Priority of vertex
  std::string group = "Default";  // group label of the vertex

  YAML::Node to_yaml() const;
  void from_yaml(const std::string& _name, const YAML::Node& data);
};

class ZoneTransitionLane
{
public:

  std::string internal_vertex;
  std::string external_vertex;

  bool is_entry_lane = true;
  bool is_exit_lane = true;

  YAML::Node to_yaml() const;
  void from_yaml(const YAML::Node& data);
};

class Zone
{
public:
  std::string name;                           // Name of the zone
  std::string entry_point;                    // Name of the vertex used as an zone entry point
  std::string external_entry_point;           // Name of the vertex used as an zone entry point from the outside
  std::string level;                          // Which level the zone is at
  std::string type;                           // What kind of zone it is

  double x = 0.0;                             // x coordinate of the zone in the global frame
  double y = 0.0;                             // y coordinate of the zone in the global frame
  double yaw = 0.0;                           // yaw of the zone in the global frame

  double width = 1.0;                         // Width of the zone in meters
  double depth = 1.0;                         // Depth of the zone in meters

  double zone_elevation = DBL_MAX;            // Zone elevation. Used to check which level the zone is at

  bool show_vertices = true;                  // Visual marker. Used to hide or show verticse in the zone
  bool show_zone = true;                      // Visual marker. Used to hide or show the zone

  double external_entry_point_x = 0.0;              // x coordinate of the external entry point in the global frame in pixels
  double external_entry_point_y = 0.0;              // y coordinate of the external entry point in the global frame in pixels

  std::vector<ZoneVertex> vertices;           // Vertices in the zone

  std::vector<ZoneTransitionLane> transition_lanes;           // Transition lanes in the zone

  ////////////////////////////////////////////////////////////////////////

  Zone();

  // Function that parse to YAML
  YAML::Node to_yaml() const;

  // Function that parse from YAML
  void from_yaml(const std::string& _name, const YAML::Node& data,
    const std::vector<Level>& levels);

  // Function to draw the zone graphically
  void draw(
    QGraphicsScene* scene,
    const double meters_per_pixel,
    const std::string& level_name,
    const double elevation,
    const bool apply_transformation = true,
    const double scale = 1.0,
    const double translate_x = 0.0,
    const double translate_y = 0.0) const;
};

#endif

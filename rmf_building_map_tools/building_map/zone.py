import numpy as np


class Zone:
    def __init__(self, yaml_node, name, transform, coordinate_system):
        self.name = name
        print(f'parsing zone {name}')

        self.level = str(yaml_node['level'])
        self.depth = float(yaml_node['depth'])
        self.width = float(yaml_node['width'])
        self.yaw = float(yaml_node['yaw'])

        # Calculating the center of the zone
        self.raw_pos = (
            float(yaml_node['x']),
            float(yaml_node['y'] * coordinate_system.y_flip_scalar()),
        )
        self.x, self.y = transform.transform_point(self.raw_pos)
        self.type = 'Default'
        self.vertices = {}
        if 'vertices' in yaml_node:
            self.vertices = self.parse_vertices(yaml_node)

        if 'transition_lanes' in yaml_node:
            self.transition_lanes = self.parse_transition_lanes(yaml_node)

    def contains_point(self, x, y):
        # Test whether a point in global coordinates lies inside the zone
        # rectangle.
        dx = x - self.x
        dy = y - self.y
        s = np.sin(self.yaw)
        c = np.cos(self.yaw)
        local_x = c * dx + s * dy
        local_y = -s * dx + c * dy
        return abs(local_x) <= self.width / 2 and abs(local_y) <= self.depth / 2

    def parse_transition_lanes(self, yaml_node):
        transition_lanes = []
        for transition_lane in yaml_node['transition_lanes']:
            internal_vertex = self.construct_unique_vertex_name(
                transition_lane['internal_vertex'], yaml_node['vertices']
            )
            transition_lanes.append(
                {
                    'internal_vertex': internal_vertex,
                    'external_vertex': transition_lane['external_vertex'],
                    'is_entry_lane': transition_lane['is_entry_lane'],
                    'is_exit_lane': transition_lane['is_exit_lane'],
                }
            )
        return transition_lanes

    def construct_unique_vertex_name(self, desired_vertex_name, vertices_yaml):
        # Defining the name of the vertices.
        # The formatting for now is:
        # <zone_name>_<vertex_name>_<group>_<priority>

        vertex_yaml = vertices_yaml[desired_vertex_name]
        return (
            self.name
            + '#'
            + vertex_yaml['group']
            + '#p'
            + str(vertex_yaml['priority'])
            + '#'
            + desired_vertex_name
        )

    def parse_vertices(self, yaml_node):
        vertices = {}
        for vertice_name, vertex_yaml in yaml_node['vertices'].items():
            # Calculating the location of vertices, transforming it from zone 
            # local coordinate to global coordinate
            x = (
                vertex_yaml['x'] * np.cos(self.yaw)
                + vertex_yaml['y'] * np.sin(self.yaw)
                + self.x
            )
            y = (
                vertex_yaml['x'] * np.sin(self.yaw)
                - vertex_yaml['y'] * np.cos(self.yaw)
                + self.y
            )
            unique_vertex_name = self.construct_unique_vertex_name(
                vertice_name, yaml_node['vertices']
            )

            p = {}
            p['name'] = unique_vertex_name

            vertices[vertice_name] = {
                'name': unique_vertex_name,
                'location': [float(x), float(y)],
                'priority': str(vertex_yaml['priority']),
                'group': vertex_yaml['group'],
            }
        return vertices

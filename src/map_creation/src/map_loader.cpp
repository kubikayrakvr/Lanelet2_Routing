#include <ros/ros.h>
#include <lanelet2_io/Io.h>

#include <bits/stdc++.h>

void loadMap(){
    lanelet::LaneletMapPtr map_=lanelet::load("/home/qb/catkin_ws/src/map_creation/maps/robotaksi.osm", lanelet::Origin({0,0}));
    for(auto point : map_.get()->pointLayer){
        point.x()=std::stold(point.attribute("local_x").value());
        point.y()=std::stold(point.attribute("local_y").value());
    }
    lanelet::write("/home/qb/catkin_ws/src/map_creation/maps/saved_map.osm", *map_);
}
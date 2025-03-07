#include <ros/ros.h>
#include <lanelet2_io/Io.h>
#include <lanelet2_core/LaneletMap.h>
#include <visualization_msgs/MarkerArray.h>
#include <lanelet2_core/primitives/LineString.h>

#include <string>

visualization_msgs::MarkerArray createMapMarkers(lanelet::LaneletMapPtr map_);

void createMap(ros::NodeHandle nh_) {
    ros::Publisher map_pub;
    std::string map_path_;
    nh_.param<std::string>("map_path", map_path_, "/home/qb/catkin_ws/src/map_creation/maps/robotaksi.osm");

    //Couldn't find a way to load it without passing a placeholder origin
    lanelet::LaneletMapPtr map_=lanelet::load("/home/qb/catkin_ws/src/map_creation/maps/saved_map.osm", lanelet::Origin({0,0}));
    
    //lanelet::projection::UtmProjector projector(lanelet::Origin({40.6451, 28.1819}));
    //map_ = lanelet::load(map_path_, projector);
    

    map_pub = nh_.advertise<visualization_msgs::MarkerArray>("map_visual", 1, true);
    ros::Rate rate(10);

    while (ros::ok()) {
        if (map_) {
            visualization_msgs::MarkerArray markers = createMapMarkers(map_);
            map_pub.publish(markers);
        }
        
        ros::spinOnce();
        rate.sleep();
    }
}

visualization_msgs::MarkerArray createMapMarkers(lanelet::LaneletMapPtr map_) {
    visualization_msgs::MarkerArray marker_array;
    int id = 0;

    for (const auto& lanelet : map_->laneletLayer) {
        visualization_msgs::Marker left_marker;
        left_marker.header.frame_id = "map";
        left_marker.header.stamp = ros::Time::now();
        left_marker.ns = "lanelet2_map";
        left_marker.id = id++;
        left_marker.type = visualization_msgs::Marker::LINE_STRIP;
        left_marker.action = visualization_msgs::Marker::ADD;
        left_marker.scale.x = 0.1;
        left_marker.color.g = 1.0; 
        left_marker.color.r = 1.0;  
        left_marker.color.a = 1.0;  

        for (const auto& point : lanelet.leftBound()) {
            geometry_msgs::Point p;
            p.x = point.x();
            p.y = point.y();
            p.z = point.z();
            left_marker.points.push_back(p);
        }
        marker_array.markers.push_back(left_marker);


        visualization_msgs::Marker right_marker;
        right_marker.header.frame_id = "map";
        right_marker.header.stamp = ros::Time::now();
        right_marker.ns = "lanelet2_map";
        right_marker.id = id++;
        right_marker.type = visualization_msgs::Marker::LINE_STRIP;
        right_marker.action = visualization_msgs::Marker::ADD;
        right_marker.scale.x = 0.1; 
        right_marker.color.g = 1.0;  
        right_marker.color.r = 1.0;  
        right_marker.color.a = 1.0;  

        for (const auto& point : lanelet.rightBound()) {
            geometry_msgs::Point p;
            p.x = point.x();
            p.y = point.y();
            p.z = point.z();
            right_marker.points.push_back(p);
        }
        marker_array.markers.push_back(right_marker);


        visualization_msgs::Marker center_marker;
        center_marker.header.frame_id = "map";
        center_marker.header.stamp = ros::Time::now();
        center_marker.ns = "lanelet2_map";
        center_marker.id = id++;
        center_marker.type = visualization_msgs::Marker::LINE_STRIP;
        center_marker.action = visualization_msgs::Marker::ADD;
        center_marker.scale.x = 0.1;  
        center_marker.color.r = 0.8;
        center_marker.color.a = 1.0;  

        for (const auto& point : lanelet.centerline()) {
            geometry_msgs::Point p;
            p.x = point.x();
            p.y = point.y();
            p.z = point.z();
            center_marker.points.push_back(p);
        }
        marker_array.markers.push_back(center_marker);
    }
    for (const auto& area : map_->areaLayer) {
        visualization_msgs::Marker stop_point;
        stop_point.header.frame_id = "map";
        stop_point.header.stamp = ros::Time::now();
        stop_point.ns = "lanelet2_map";
        stop_point.id = id++;;
        stop_point.type = visualization_msgs::Marker::LINE_STRIP;
        stop_point.action = visualization_msgs::Marker::ADD;
        stop_point.scale.x = 0.1; 
        stop_point.color.b = 1.0;  
        stop_point.color.a = 1.0;
        std::vector<lanelet::ConstLineString3d> outerBound(area.outerBound());
        for(const auto& linestring : outerBound){
            for(const auto& point : linestring){
                geometry_msgs::Point p;
                p.x = point.x();
                p.y = point.y();
                p.z = point.z();
                stop_point.points.push_back(p);
            }
        }
        marker_array.markers.push_back(stop_point);
    }
    return marker_array;
}
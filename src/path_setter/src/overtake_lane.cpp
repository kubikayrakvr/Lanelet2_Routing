#include <ros/ros.h>

#include <lanelet2_io/Io.h>
#include <lanelet2_core/LaneletMap.h>
#include <lanelet2_core/primitives/LineString.h>
#include <lanelet2_core/geometry/Lanelet.h>
#include <lanelet2_core/primitives/Point.h>

#include <geometry_msgs/PointStamped.h>
#include <std_msgs/Int64.h>
#include <nav_msgs/Path.h>

void overtakeCallback(const geometry_msgs::PointStamped::ConstPtr& msg);
void pathCallback(const nav_msgs::Path::ConstPtr& msg);
lanelet::Id nearestLane(lanelet::LaneletMapPtr map_, const geometry_msgs::PointStamped& overtake_msg, const nav_msgs::Path& path_msg);

geometry_msgs::PointStamped overtake_msg;
nav_msgs::Path path_msg;
bool received_overtake;

void findOvertake(ros::NodeHandle nh_) {
    ros::Publisher overtake_pub = nh_.advertise<std_msgs::Int64>("overtake_id", 1);
    ros::Subscriber overtake_sub = nh_.subscribe("clicked_point", 1, overtakeCallback);
    ros::Subscriber path_sub = nh_.subscribe("shortest_path", 1, pathCallback);
    lanelet::LaneletMapPtr map_ = lanelet::load("/home/qb/catkin_ws/src/map_creation/maps/saved_map.osm", lanelet::Origin({0, 0}));

    lanelet::Id current_lane_id;
    std_msgs::Int64 id_msg;

    ros::Rate loop_rate(10);

    while(ros::ok()){
        if(received_overtake){
            current_lane_id=nearestLane(map_, overtake_msg, path_msg);
            ROS_INFO("Nearest Chosen Overtake Lanelet Id: %ld", current_lane_id);
            received_overtake = false;
        }
        id_msg.data=current_lane_id;
        overtake_pub.publish(id_msg);
        ros::spinOnce(); 
        loop_rate.sleep();
    }
}

void overtakeCallback(const geometry_msgs::PointStamped::ConstPtr& msg)
{
    received_overtake=true;
    overtake_msg = *msg;
    double x = msg->point.x;
    double y = msg->point.y;
    double z = msg->point.z;
    ROS_INFO("Received position - Position: [x = %f, y = %f, z = %f]", x, y, z);
}

void pathCallback(const nav_msgs::Path::ConstPtr& msg){
    path_msg = *msg;
}

lanelet::Id nearestLane(lanelet::LaneletMapPtr map_, const geometry_msgs::PointStamped& overtake_msg, const nav_msgs::Path& path_msg) {
    lanelet::BasicPoint2d overtake_point(overtake_msg.point.x, overtake_msg.point.y);

    double min_distance = std::numeric_limits<double>::max();
    lanelet::BasicPoint2d nearest_path_point;
    for (const auto& pose : path_msg.poses) {
        lanelet::BasicPoint2d path_point(pose.pose.position.x, pose.pose.position.y);
        double distance = lanelet::geometry::distance(path_point, overtake_point);
        if (distance < min_distance) {
            min_distance = distance;
            nearest_path_point = path_point;
        }
    }

    lanelet::Id nearest_lanelet_id;
    min_distance = std::numeric_limits<double>::max();
    for (const auto& lanelet : map_->laneletLayer) {
        double distance = lanelet::geometry::distance2d(lanelet, nearest_path_point);
        if (distance < min_distance) {
            min_distance = distance;
            nearest_lanelet_id = lanelet.id();
        }
    }

    return nearest_lanelet_id;
}

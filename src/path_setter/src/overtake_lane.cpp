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
bool receivedOvertake;

void calculateOvertake(ros::NodeHandle nh_) {
    ros::Publisher overtake_pub = nh_.advertise<std_msgs::Int64>("overtake_id", 1);
    ros::Subscriber overtake_sub = nh_.subscribe("clicked_point", 1, overtakeCallback);
    ros::Subscriber path_sub = nh_.subscribe("shortest_path", 1, pathCallback);
    lanelet::LaneletMapPtr map_ = lanelet::load("/home/qb/catkin_ws/src/map_creation/maps/saved_map.osm", lanelet::Origin({0, 0}));

    lanelet::Id currentLaneId;
    std_msgs::Int64 id_msg;

    ros::Rate loop_rate(10);

    while(ros::ok()){
        if(receivedOvertake){
            currentLaneId=nearestLane(map_, overtake_msg, path_msg);
            ROS_INFO("Nearest Chosen Overtake Lanelet Id: %ld", currentLaneId);
            receivedOvertake = false;
        }
        id_msg.data=currentLaneId;
        overtake_pub.publish(id_msg);
        ros::spinOnce(); 
        loop_rate.sleep();
    }
}

void overtakeCallback(const geometry_msgs::PointStamped::ConstPtr& msg)
{
    receivedOvertake=true;
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
    lanelet::BasicPoint2d overtakePoint(overtake_msg.point.x, overtake_msg.point.y);

    double minDistance = std::numeric_limits<double>::max();
    lanelet::BasicPoint2d nearestPathPoint;
    for (const auto& pose : path_msg.poses) {
        lanelet::BasicPoint2d pathPoint(pose.pose.position.x, pose.pose.position.y);
        double distance = lanelet::geometry::distance(pathPoint, overtakePoint);
        if (distance < minDistance) {
            minDistance = distance;
            nearestPathPoint = pathPoint;
        }
    }

    lanelet::Id nearestLaneletId;
    minDistance = std::numeric_limits<double>::max();
    for (const auto& lanelet : map_->laneletLayer) {
        double distance = lanelet::geometry::distance2d(lanelet, nearestPathPoint);
        if (distance < minDistance) {
            minDistance = distance;
            nearestLaneletId = lanelet.id();
        }
    }

    return nearestLaneletId;
}

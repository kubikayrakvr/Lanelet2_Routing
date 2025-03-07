#include <ros/ros.h>

#include <lanelet2_io/Io.h>
#include <lanelet2_core/LaneletMap.h>
#include <lanelet2_core/primitives/LineString.h>
#include <lanelet2_core/geometry/Lanelet.h>
#include <lanelet2_core/primitives/Point.h>

#include <geometry_msgs/PoseWithCovarianceStamped.h>
#include <std_msgs/Int64.h>

void startCallback(const geometry_msgs::PoseWithCovarianceStamped::ConstPtr& msg);
lanelet::Id nearestLane(lanelet::LaneletMapPtr map_, const geometry_msgs::PoseWithCovarianceStamped& start_msg);
lanelet::Id findDock(lanelet::LaneletMapPtr map_, const geometry_msgs::PoseWithCovarianceStamped& start_msg);

geometry_msgs::PoseWithCovarianceStamped start_msg;
bool receivedStart;
bool startInside;

void calculateStart(ros::NodeHandle nh_) {
    ros::Publisher start_pub = nh_.advertise<std_msgs::Int64>("start_id", 1);
    ros::Publisher dock_pub = nh_.advertise<std_msgs::Int64>("dock_id", 1);
    ros::Subscriber sub = nh_.subscribe("initialpose", 1, startCallback);

    lanelet::LaneletMapPtr map_ = lanelet::load("/home/qb/catkin_ws/src/map_creation/maps/saved_map.osm", lanelet::Origin({0, 0}));

    lanelet::Id currentLaneId;
    std_msgs::Int64 id_msg;
    std_msgs::Int64 dock_msg;
    
    ros::Rate loop_rate(10);
    
    while(ros::ok()){
        if(receivedStart){
            currentLaneId=nearestLane(map_, start_msg);
            if(startInside){
                ROS_INFO("Chosen Start Lanelet Id: %ld", currentLaneId);
            }
            else{
                ROS_INFO("Nearest Chosen Start Lanelet Id: %ld", currentLaneId);
            }
            receivedStart=false;
            startInside=false;
        }
        dock_msg.data=findDock(map_, start_msg);
        id_msg.data=currentLaneId;
        start_pub.publish(id_msg);
        dock_pub.publish(dock_msg);
        loop_rate.sleep();
        ros::spinOnce(); 
    }
}

void startCallback(const geometry_msgs::PoseWithCovarianceStamped::ConstPtr& msg)
{
    receivedStart=true;
    start_msg = *msg;
    double x = msg->pose.pose.position.x;
    double y = msg->pose.pose.position.y;
    double z = msg->pose.pose.position.z;
    ROS_INFO("Received position - Position: [x = %f, y = %f, z = %f]", x, y, z);
}

lanelet::Id nearestLane(lanelet::LaneletMapPtr map_, const geometry_msgs::PoseWithCovarianceStamped& start_msg){
    lanelet::BasicPoint3d receivedPoint(start_msg.pose.pose.position.x, start_msg.pose.pose.position.y, start_msg.pose.pose.position.z);
    double distance;
    double minDistance = std::numeric_limits<double>::max();
    lanelet::Id minDistanceId;
    for(const auto& lanelet : map_->laneletLayer){
        lanelet::BasicPoint2d receivedPoint2d(receivedPoint.x(), receivedPoint.y());
        if(boost::geometry::covered_by(receivedPoint2d, lanelet.polygon2d())){
            minDistanceId=lanelet.id();
            startInside=true;
            break;
        }
        else{
            distance=lanelet::geometry::distanceToCenterline3d (lanelet, receivedPoint);
            if(distance<minDistance){
                minDistance=distance;
                minDistanceId=lanelet.id();
            }
        }
    }
    return minDistanceId;
}

lanelet::Id findDock(lanelet::LaneletMapPtr map_, const geometry_msgs::PoseWithCovarianceStamped& start_msg){
    lanelet::BasicPoint2d receivedPoint(start_msg.pose.pose.position.x, start_msg.pose.pose.position.y);
    auto nearestDock = lanelet::geometry::findNearest(map_->areaLayer, receivedPoint, 1);
    return nearestDock[0].second.id();
}
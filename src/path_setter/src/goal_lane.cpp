#include <ros/ros.h>

#include <lanelet2_io/Io.h>
#include <lanelet2_core/LaneletMap.h>
#include <lanelet2_core/primitives/LineString.h>
#include <lanelet2_core/geometry/Lanelet.h>
#include <lanelet2_core/primitives/Point.h>

#include <geometry_msgs/PoseStamped.h>
#include <std_msgs/Int64.h>

void goalCallback(const geometry_msgs::PoseStamped::ConstPtr& msg);
lanelet::Id nearestLane(lanelet::LaneletMapPtr map_, const geometry_msgs::PoseStamped& goal_msg);

bool goalInside;
bool receivedGoal;
geometry_msgs::PoseStamped goal_msg;

void calculateGoal(ros::NodeHandle nh_) {
    ros::Publisher goal_pub = nh_.advertise<std_msgs::Int64>("goal_id", 1);
    ros::Subscriber sub = nh_.subscribe("move_base_simple/goal", 1, goalCallback);

    lanelet::LaneletMapPtr map_ = lanelet::load("/home/qb/catkin_ws/src/map_creation/maps/saved_map.osm", lanelet::Origin({0, 0}));

    lanelet::Id currentLaneId;
    std_msgs::Int64 id_msg;

    ros::Rate loop_rate(10);

    while(ros::ok()){
        if(receivedGoal){
            currentLaneId=nearestLane(map_, goal_msg);
            if(goalInside){
                ROS_INFO("Chosen Goal Lanelet Id: %ld", currentLaneId);
            }
            else{
                ROS_INFO("Nearest Chosen Goal Lanelet Id: %ld", currentLaneId);
            }
            receivedGoal=false;
            goalInside=false;
        }
        id_msg.data=currentLaneId;
        goal_pub.publish(id_msg);
        loop_rate.sleep();
        ros::spinOnce(); 
    }
}

void goalCallback(const geometry_msgs::PoseStamped::ConstPtr& msg)
{
    receivedGoal=true;
    goal_msg = *msg;
    double x = msg->pose.position.x;
    double y = msg->pose.position.y;
    double z = msg->pose.position.z;
    ROS_INFO("Received position - Position: [x = %f, y = %f, z = %f]", x, y, z);
}

lanelet::Id nearestLane(lanelet::LaneletMapPtr map_, const geometry_msgs::PoseStamped& goal_msg){
    lanelet::BasicPoint3d receivedPoint(goal_msg.pose.position.x, goal_msg.pose.position.y, goal_msg.pose.position.z);
    double distance;
    double minDistance = std::numeric_limits<double>::max();
    lanelet::Id minDistanceId;
    for(const auto& lanelet : map_->laneletLayer){
        lanelet::BasicPoint2d receivedPoint2d(receivedPoint.x(), receivedPoint.y());
        if(boost::geometry::covered_by(receivedPoint2d, lanelet.polygon2d())){
            minDistanceId=lanelet.id();
            goalInside=true;
            break;
        }
        else{
            distance = lanelet::geometry::distanceToCenterline3d (lanelet, receivedPoint);
            if(distance<minDistance){
                minDistance=distance;
                minDistanceId=lanelet.id();
            }
        }
    }
    return minDistanceId;
}

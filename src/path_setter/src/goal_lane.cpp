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

bool goal_inside;
bool received_goal;
geometry_msgs::PoseStamped goal_msg;

void findGoal(ros::NodeHandle nh_) {
    ros::Publisher goal_pub = nh_.advertise<std_msgs::Int64>("goal_id", 1);
    ros::Subscriber sub = nh_.subscribe("move_base_simple/goal", 1, goalCallback);
    //Update this
    lanelet::LaneletMapPtr map_ = lanelet::load("/home/qb/catkin_ws/src/map_creation/maps/saved_map.osm", lanelet::Origin({0, 0}));

    lanelet::Id current_lane_id;
    std_msgs::Int64 id_msg;

    ros::Rate loop_rate(10);

    while(ros::ok()){
        if(received_goal){
            current_lane_id=nearestLane(map_, goal_msg);
            if(goal_inside){
                ROS_INFO("Chosen Goal Lanelet Id: %ld", current_lane_id);
            }
            else{
                ROS_INFO("Nearest Chosen Goal Lanelet Id: %ld", current_lane_id);
            }
            received_goal=false;
            goal_inside=false;
        }
        id_msg.data=current_lane_id;
        goal_pub.publish(id_msg);
        loop_rate.sleep();
        ros::spinOnce(); 
    }
}

void goalCallback(const geometry_msgs::PoseStamped::ConstPtr& msg)
{
    received_goal=true;
    goal_msg = *msg;
    double x = msg->pose.position.x;
    double y = msg->pose.position.y;
    double z = msg->pose.position.z;
    ROS_INFO("Received position - Position: [x = %f, y = %f, z = %f]", x, y, z);
}

lanelet::Id nearestLane(lanelet::LaneletMapPtr map_, const geometry_msgs::PoseStamped& goal_msg){
    lanelet::BasicPoint3d received_point(goal_msg.pose.position.x, goal_msg.pose.position.y, goal_msg.pose.position.z);
    double distance;
    double min_distance = std::numeric_limits<double>::max();
    lanelet::Id min_distance_id;
    for(const auto& lanelet : map_->laneletLayer){
        lanelet::BasicPoint2d received_point2d(received_point.x(), received_point.y());
        if(boost::geometry::covered_by(received_point2d, lanelet.polygon2d())){
            min_distance_id=lanelet.id();
            goal_inside=true;
            break;
        }
        else{
            distance = lanelet::geometry::distanceToCenterline3d (lanelet, received_point);
            if(distance<min_distance){
                min_distance=distance;
                min_distance_id=lanelet.id();
            }
        }
    }
    return min_distance_id;
}

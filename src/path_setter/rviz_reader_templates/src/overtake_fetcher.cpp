#include <ros/ros.h>
#include <geometry_msgs/PointStamped.h>

void positionCallback(const geometry_msgs::PointStamped::ConstPtr& msg)
{
    double x = msg->point.x;
    double y = msg->point.y;
    double z = msg->point.z;
    ROS_INFO("Received overtake point - Position: [x = %f, y = %f, z = %f]", x, y, z);
    
}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "overtake_fetcher");
    ros::NodeHandle nh;
    ros::Subscriber sub = nh.subscribe("clicked_point", 1000, positionCallback);
    ros::spin();
    return 0;
}

#include <ros/ros.h>

#include <std_msgs/Int64.h>
#include <nav_msgs/Path.h>
#include <geometry_msgs/PoseStamped.h>

#include <lanelet2_core/LaneletMap.h>
#include <lanelet2_io/Io.h>
#include <lanelet2_routing/Route.h>
#include <lanelet2_routing/RoutingGraph.h>
#include <lanelet2_routing/LaneletPath.h>
#include <lanelet2_traffic_rules/TrafficRulesFactory.h>
#include <lanelet2_core/geometry/Area.h>
#include <lanelet2_core/geometry/Lanelet.h>

#include <vector>

lanelet::Id start_id = 0;
lanelet::Id dock_id = 0;
lanelet::Id goal_id = 0;
lanelet::Id overtake_id = 0;

lanelet::routing::LaneletPath findPath(lanelet::LaneletMapPtr map_, const lanelet::Id &current_start, const lanelet::Id &current_overtake);
lanelet::routing::LaneletPath overtakePath(lanelet::LaneletMapPtr map_, lanelet::routing::LaneletPath currentPath);
nav_msgs::Path drawPath(lanelet::routing::LaneletPath currentPath);
lanelet::routing::LaneletPath dockingPath(lanelet::LaneletMapPtr map_, lanelet::routing::LaneletPath currentPath);
lanelet::LineString3d FourPointBezier(
    lanelet::ConstPoint3d p0,
    lanelet::ConstPoint3d p1,
    lanelet::ConstPoint3d p2,
    lanelet::ConstPoint3d p3
);
lanelet::LineString3d createRightBound(lanelet::LineString3d leftBound);

void retrieveStart(const std_msgs::Int64::ConstPtr& id_msg){
    start_id=id_msg->data;
}

void retrieveDock(const std_msgs::Int64::ConstPtr& id_msg){
    dock_id=id_msg->data;
}

void retrieveGoal(const std_msgs::Int64::ConstPtr& id_msg){
    goal_id=id_msg->data;
}

void retrieveOvertake(const std_msgs::Int64::ConstPtr& id_msg){
    overtake_id=id_msg->data;
}

void setPath(ros::NodeHandle nh_){
    ros::Publisher route_pub;
    ros::Subscriber start_sub = nh_.subscribe("start_id", 1, retrieveStart);
    ros::Subscriber goal_sub = nh_.subscribe("goal_id", 1, retrieveGoal);
    ros::Subscriber overtake_sub = nh_.subscribe("overtake_id", 1, retrieveOvertake);
    ros::Subscriber dock_sub = nh_.subscribe("dock_id", 1, retrieveDock);
    std::string map_path_;
    lanelet::Id current_start = 0;
    lanelet::Id current_goal = 0;
    lanelet::Id current_overtake = 0;
    lanelet::routing::LaneletPath currentPath;
 
    lanelet::LaneletMapPtr map_ = lanelet::load("/home/qb/catkin_ws/src/map_creation/maps/saved_map.osm", lanelet::Origin({0, 0}));
    route_pub = nh_.advertise<nav_msgs::Path>("shortest_path", 1, true);

    while (ros::ok()) {
        if (map_->laneletLayer.exists(start_id)){
            if(goal_id != current_goal || start_id != current_start){
                currentPath = findPath(map_, current_start, current_overtake);
                currentPath = dockingPath(map_, currentPath);
                current_goal = goal_id;
                current_start = start_id;
            }
            if(overtake_id != current_overtake){
                if(map_->laneletLayer.exists(overtake_id) && overtake_id != 0){
                    currentPath = overtakePath(map_, currentPath);
                    current_overtake = overtake_id;
                }
            }
            nav_msgs::Path path = drawPath(currentPath);
            route_pub.publish(path);
        }
        ros::spinOnce();
    }
}

lanelet::routing::LaneletPath findPath(lanelet::LaneletMapPtr map_, const lanelet::Id &current_start, const lanelet::Id &current_overtake){
    lanelet::traffic_rules::TrafficRulesPtr trafficRules = 
    lanelet::traffic_rules::TrafficRulesFactory::create(lanelet::Locations::Germany, lanelet::Participants::Vehicle);
    lanelet::routing::RoutingGraphUPtr routingGraph = lanelet::routing::RoutingGraph::build(*map_, *trafficRules);
    lanelet::ConstLanelet lanelet = map_->laneletLayer.get(start_id);
    lanelet::ConstLanelet toLanelet;
    //The first condition is for the nearest docking route when the goal is not refreshed if a new start is selected.
    if(current_start == start_id && map_->laneletLayer.exists(goal_id)){
        toLanelet = map_->laneletLayer.get(goal_id);
    }
    
    else{
        for(const auto& lanelet : map_->laneletLayer){
            if(lanelet::geometry::overlaps2d(map_->areaLayer.get(dock_id), lanelet)){
                toLanelet = lanelet;
                break;
            }
        }
              
    }
    lanelet::Optional<lanelet::routing::Route> route = routingGraph->getRoute(lanelet, toLanelet, 0);
    lanelet::routing::LaneletPath currentPath = route->shortestPath();
    return currentPath;
}

lanelet::routing::LaneletPath dockingPath(lanelet::LaneletMapPtr map_, lanelet::routing::LaneletPath currentPath) {
    std::vector<lanelet::ConstLanelet> pathVector;
    
    for(const auto& lanelet : currentPath) {
        bool needsDocking = false;
        lanelet::Id alreadyDocked; 
        for(const auto& area : map_->areaLayer) {
            if(lanelet::geometry::overlaps2d(area, lanelet) && area.id() != alreadyDocked){
                needsDocking = true;
                alreadyDocked = area.id();
                std::vector<lanelet::ConstLineString3d> outerBounds = area.outerBound();
                
                lanelet::ConstLineString3d areaLinestring = outerBounds[0];
                lanelet::ConstPoint3d p0 = areaLinestring[1];
                lanelet::ConstPoint3d p1 = areaLinestring[2];
                lanelet::ConstPoint3d p2 = areaLinestring[3];
                lanelet::ConstPoint3d p3 = areaLinestring[4];
                
                lanelet::LineString3d leftBound = FourPointBezier(p0, p1, p2, p3);

                lanelet::LineString3d rightBound = createRightBound(leftBound);

                //I Do Not Know Why The Left Bound Needs To Be The Bezier Curve But It Just Works So I Dont Question It, Must Be The Order Of The Points
                pathVector.push_back(lanelet::Lanelet(lanelet::utils::getId(), leftBound, rightBound).invert());
                break;
            }
        }
        if (!needsDocking) {
            pathVector.push_back(lanelet);
        }
    }
    
    return lanelet::routing::LaneletPath(pathVector);
}

lanelet::routing::LaneletPath overtakePath(lanelet::LaneletMapPtr map_, lanelet::routing::LaneletPath currentPath){
    if(map_->laneletLayer.exists(overtake_id) && overtake_id != 0){
        lanelet::ConstLanelet overtakeLanelet = map_->laneletLayer.get(overtake_id);
        auto leftLanelet = map_->laneletLayer.findUsages(overtakeLanelet.leftBound().invert());
        if(!leftLanelet.empty()){
            std::vector<lanelet::ConstLanelet> pathVector;
            for(const auto& lanelet : currentPath){
                if(lanelet.id() != overtake_id){
                    pathVector.push_back(lanelet);
                }
                else{
                    lanelet::Lanelet inverted = leftLanelet[0].invert();
                    pathVector.push_back(inverted);
                }
            }
            lanelet::routing::LaneletPath overtakePath(pathVector);
            currentPath = overtakePath;
        }
        else{
            ROS_WARN("Invalid Left Lane");
        }
    }
    return currentPath;
}
nav_msgs::Path drawPath(lanelet::routing::LaneletPath currentPath){
    nav_msgs::Path drawnPath;
    drawnPath.header.frame_id = "map";
    drawnPath.header.stamp = ros::Time::now();
    for(const auto& path : currentPath){
        for(const auto& point : path.centerline()){
            geometry_msgs::PoseStamped pathPoint;
            pathPoint.header.stamp = ros::Time::now();
            pathPoint.pose.position.x = point.x();
            pathPoint.pose.position.y = point.y();
            pathPoint.pose.position.z = point.z();
            drawnPath.poses.push_back(pathPoint);
        }
    }
    return drawnPath;
}

lanelet::LineString3d FourPointBezier(
    lanelet::ConstPoint3d p0,
    lanelet::ConstPoint3d p1,
    lanelet::ConstPoint3d p2,
    lanelet::ConstPoint3d p3
){
    lanelet::LineString3d curveLineString;
    std::vector<Eigen::Vector3d> controlPoints = {
        Eigen::Vector3d(p0.x(), p0.y(), p0.z()),
        Eigen::Vector3d(p1.x(), p1.y(), p1.z()),
        Eigen::Vector3d(p2.x(), p2.y(), p2.z()),
        Eigen::Vector3d(p3.x(), p3.y(), p3.z())
    };
    
    int numPoints = 100;
    for (double t = 0; t <= 1.0; t += 1.0/numPoints) {
        double t2 = t * t;
        double t3 = t2 * t;
        double mt = 1 - t;
        double mt2 = mt * mt;
        double mt3 = mt2 * mt;
        
        double b0 = mt3;
        double b1 = 3 * mt2 * t;
        double b2 = 3 * mt * t2;
        double b3 = t3;
        
        Eigen::Vector3d point = 
            b0 * controlPoints[0] +
            b1 * controlPoints[1] +
            b2 * controlPoints[2] +
            b3 * controlPoints[3];
        
        curveLineString.push_back(lanelet::Point3d(lanelet::utils::getId(), point.x(), point.y(), point.z()));
    }
    return curveLineString;
}

lanelet::LineString3d createRightBound(lanelet::LineString3d leftBound){
    lanelet::LineString3d rightBound(lanelet::utils::getId());
    for (size_t i = 0; i < leftBound.size(); i++) {
        Eigen::Vector2d direction;
        if (i < leftBound.size() - 1) {
            direction = Eigen::Vector2d(
                leftBound[i + 1].x() - leftBound[i].x(),
                leftBound[i + 1].y() - leftBound[i].y()
            );
        } else {
            direction = Eigen::Vector2d(
                leftBound[i].x() - leftBound[i-1].x(),
                leftBound[i].y() - leftBound[i-1].y()
            );
        }
        
        direction.normalize();
        Eigen::Vector2d normal(-direction.y(), direction.x());
        
        rightBound.push_back(lanelet::Point3d(
            lanelet::utils::getId(),
            //Width
            leftBound[i].x() - normal.x() * 3.0,
            //Lenght
            leftBound[i].y() - normal.y() * 2.0,
            leftBound[i].z()
        ));
    }
    return rightBound;
}

//NOTE: The Fact That The Middle Docking Point Is Adjacent To Two Lanelets Is Really Problematic
//THIS CODE IS STUPID I AINT GONNA LIE BUT IT WORKS EXCEPT ONE POINT WHICH IS 1/3 OF THE POINTS WHICH IS QUITE A LARGE PERCENTAGE
//PLUS IT FIND THE CLOSEST IN TERMS OF COORDINATES NOT PATH WISE THATS ALSO STUPID
//I PROBABLY WILL SCRAP THIS CODE ANYWAYS UGH
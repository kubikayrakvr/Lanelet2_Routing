Lanelet2_Routing

This repository includes files for the creation of map visuals and path creation in rviz based on pre-existing osm files. The path creation is based on chosen points through rviz.
Path Creation Features

    Finding and publishing the path to the closest docking area if only a starting point is given.
    Finding and publishing the closest path between start and goal points.
    Docking into any docking areas encountered on the path.
    Creating a overtake point based on a published point.

To use this code a lanelet map has to be placed and the map path has to be specified in the code to run properly.
Also make sure your lanelet2 library is linked properly.
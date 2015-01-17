cmake_minimum_required(VERSION 2.8)
project(wuzlcam)
find_package( OpenCV REQUIRED )

add_executable(wuzlcam main.cpp Table.cpp BallPosition.cpp BallPositions.cpp Bar.cpp Event.cpp)
target_link_libraries( wuzlcam ${OpenCV_LIBS} )

install(TARGETS wuzlcam RUNTIME DESTINATION bin)

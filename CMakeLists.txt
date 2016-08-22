set (CMAKE_INSTALL_PREFIX /usr)

cmake_minimum_required(VERSION 3.0.0)

project (WvDialer)

find_package(KF5Auth          REQUIRED)
find_package(KF5Notifications REQUIRED)
add_subdirectory(src)

install ( FILES       ${PROJECT_NAME}.desktop
          DESTINATION ${SHARE_INSTALL_PREFIX}/applications )

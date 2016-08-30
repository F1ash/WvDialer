
set(wvdialer_helper_SRCS
    wvdialer_helper.cpp)

find_package(Qt5 COMPONENTS DBus REQUIRED)
qt5_wrap_cpp (HELPER_MOC_SOURCES wvdialer_helper.h)

set (
    HELPER_BUILD_PROJECT_LIBRARIES
    ${Qt5DBus_LIBRARIES}
    KF5::Auth
    KF5::Notifications)
add_definitions(
    ${Qt5Widgets_DEFINITIONS}
    ${Qt5DBus_DEFINITIONS})

add_executable(wvdialer_helper ${wvdialer_helper_SRCS} ${HELPER_MOC_SOURCES})
target_link_libraries(wvdialer_helper ${HELPER_BUILD_PROJECT_LIBRARIES})
install(TARGETS wvdialer_helper DESTINATION ${KAUTH_HELPER_INSTALL_DIR})
#message("KAUTH_HELPER_INSTALL_DIR: ${KAUTH_HELPER_INSTALL_DIR}")
#message("KAUTH_POLICY_FILES_INSTALL_DIR: ${KAUTH_POLICY_FILES_INSTALL_DIR}")
#message("DBUS_SYSTEM_SERVICES_INSTALL_DIR: ${DBUS_SYSTEM_SERVICES_INSTALL_DIR}")

kauth_install_helper_files(
    wvdialer_helper
    pro.russianfedora.wvdialer
    root)
kauth_install_actions(
    pro.russianfedora.wvdialer
    actions.actions)

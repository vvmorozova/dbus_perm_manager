#include "system_time.h"

SystemTime::SystemTime() {
  const char *serviceName = "com.system.time";
  connection = sdbus::createSessionBusConnection(serviceName);

  sdbus::ObjectPath objectPath{"/"};
  dbusObject = sdbus::createObject(*connection, std::move(objectPath));

  dbusObject->registerMethod("GetSystemTime")
      .onInterface("com.system.time")
      .withInputParamNames()
      .implementedAs(std::move([this]() { GetSystemTime(); }));

  dbusObject->finishRegistration();
}

void SystemTime::start() { connection->enterEventLoop(); }

uint64_t SystemTime::GetSystemTime() {

  std::string path = getPath(dbusObject, connection);
  std::cout << "Path " << path << std::endl;

  // ask permission

  sdbus::ObjectPath objectPath{"/"};
  auto proxy =
      sdbus::createProxy("com.system.permissions", std::move(objectPath));

  bool result = false;
  proxy->callMethod("CheckApplicationHasPermission")
      .onInterface("com.system.permissions")
      .withArguments(path, PermissionManager::Permissions::SystemTime)
      .storeResultsTo(result);

  if (!result) {
    throw sdbus::Error("com.system.permissions.Error.UnathorizedAccess",
                       "Time permissions was no granted to " + path);
    return 0;
  }
  // get sys time
  auto currTime =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

  std::cout << "curr time " << std::ctime(&currTime) << std::endl;
  return currTime;
}
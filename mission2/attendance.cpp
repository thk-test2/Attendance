#pragma once

#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

constexpr int ATTEND_HISTORY_CNT = 500;
constexpr int MAX_USER_CNT = 100;
constexpr int DAYS_OF_WEEK = 7;

enum DayOfWeek {
  MONDAY,
  TUESDAY,
  WEDNESDAY,
  THURSDAY,
  FRIDAY,
  SATURDAY,
  SUNDAY
};

struct IdInfo {
  std::map<std::string, int> idTable;
  int totalUsers{0};
};

struct UserInfo {
  std::string name{};
  std::string grade{};
  int attendHistory[DAYS_OF_WEEK]{0};
  int totalPoints{0};
  bool specialDayAttended{false};
};

class IPolicy {
 public:
  virtual bool isSpecialDay(int dayIndex) = 0;
  virtual int getDayPoints(int dayIndex) = 0;
  virtual int bonusPointForTrainingDay(const UserInfo& userInfo) = 0;
  virtual int bonusPointForWeekends(const UserInfo& userInfo) = 0;
  virtual std::string getUserGrade(int totalPoints) = 0;
  virtual void evaluateGrade(UserInfo& userInfo) = 0;
  virtual bool isRemovedPlayer(const UserInfo& userInfo) = 0;
};

class CurrentPolicy : public IPolicy {
 public:
  bool isSpecialDay(int dayIndex) override {
    if (dayIndex == DayOfWeek::WEDNESDAY || dayIndex == DayOfWeek::SATURDAY ||
        dayIndex == DayOfWeek::SUNDAY)
      return true;
    return false;
  }

  int getDayPoints(int dayIndex) override {
    if (dayIndex == DayOfWeek::WEDNESDAY)
      return TRAININGDAY_POINT;
    else if ((dayIndex == DayOfWeek::SATURDAY) ||
             (dayIndex == DayOfWeek::SUNDAY))
      return WEEKENDS_POINT;
    return NORMALDAY_POINT;
  }

  int bonusPointForTrainingDay(const UserInfo& userInfo) override {
    if (userInfo.attendHistory[DayOfWeek::WEDNESDAY] >=
        BONUS_ATTEND_DAY_THRESHOLD)
      return BONUS_POINT;
    return 0;
  }

  int bonusPointForWeekends(const UserInfo& userInfo) override {
    if ((userInfo.attendHistory[DayOfWeek::SATURDAY] +
         userInfo.attendHistory[DayOfWeek::SUNDAY]) >=
        BONUS_ATTEND_DAY_THRESHOLD)
      return BONUS_POINT;
    return 0;
  }

  std::string getUserGrade(int totalPoints) override {
    if (totalPoints >= GOLD_GRADE_THRESHOLD)
      return GOLD_TEXT;
    else if (totalPoints >= SILVER_GRADE_THRESHOLD)
      return SILVER_TEXT;
    return NORMAL_TEXT;
  }

  void evaluateGrade(UserInfo& userInfo) override {
    userInfo.totalPoints += bonusPointForTrainingDay(userInfo);
    userInfo.totalPoints += bonusPointForWeekends(userInfo);
    userInfo.grade = getUserGrade(userInfo.totalPoints);
  }

  bool isRemovedPlayer(const UserInfo& userInfo) override {
    return userInfo.grade == NORMAL_TEXT && userInfo.specialDayAttended == false;
  }

 private:
  const int NORMALDAY_POINT = 1;
  const int WEEKENDS_POINT = 2;
  const int TRAININGDAY_POINT = 3;
  const int BONUS_POINT = 10;

  const int BONUS_ATTEND_DAY_THRESHOLD = 10;
  const int GOLD_GRADE_THRESHOLD = 50;
  const int SILVER_GRADE_THRESHOLD = 30;

  const std::string GOLD_TEXT = "GOLD";
  const std::string SILVER_TEXT = "SILVER";
  const std::string NORMAL_TEXT = "NORMAL";
};

class AttendanceSystem {
 public:
  AttendanceSystem(IPolicy* _policy) : policy(_policy) {}

  bool isNewUser(const std::string& userName, IdInfo& idInfo);
  void addNewUser(IdInfo& idInfo, const std::string& userName,
                  std::array<UserInfo, MAX_USER_CNT>& userInfoList);

  int getUniformId(const std::string& userName, IdInfo& idInfo);
  int getDayIndex(const std::string& dayOfWeek);

  void analyzeEachRecord(const std::string& userName,
                         const std::string& dayOfWeek, IdInfo& idInfo,
                         std::array<UserInfo, MAX_USER_CNT>& userInfoList);
  void analyzeAllRecord(IdInfo& idInfo,
                        std::array<UserInfo, MAX_USER_CNT>& userInfoList);
  void analyzeAndPrintUserInfo();

  void printUserInfo(const UserInfo& userInfo);
  void printRemovedUser(const IdInfo& idInfo,
                        std::array<UserInfo, MAX_USER_CNT>& userInfoList);
  void checkIfDebugData(const std::string& userName);

 private:
  IPolicy* policy;
};

bool AttendanceSystem::isNewUser(const std::string& userName, IdInfo& idInfo) {
  return idInfo.idTable.count(userName) == 0;
}

void AttendanceSystem::addNewUser(
    IdInfo& idInfo, const std::string& userName,
    std::array<UserInfo, MAX_USER_CNT>& userInfoList) {
  idInfo.idTable.insert({userName, ++(idInfo.totalUsers)});
  userInfoList.at(idInfo.totalUsers).name = userName;
}

int AttendanceSystem::getUniformId(const std::string& userName,
                                   IdInfo& idInfo) {
  return idInfo.idTable[userName];
}

int AttendanceSystem::getDayIndex(const std::string& dayOfWeek) {
  if (dayOfWeek == "monday") return DayOfWeek::MONDAY;
  if (dayOfWeek == "tuesday") return DayOfWeek::TUESDAY;
  if (dayOfWeek == "wednesday") return DayOfWeek::WEDNESDAY;
  if (dayOfWeek == "thursday") return DayOfWeek::THURSDAY;
  if (dayOfWeek == "friday") return DayOfWeek::FRIDAY;
  if (dayOfWeek == "saturday") return DayOfWeek::SATURDAY;
  if (dayOfWeek == "sunday") return DayOfWeek::SUNDAY;
}

void AttendanceSystem::analyzeEachRecord(
    const std::string& userName, const std::string& dayOfWeek, IdInfo& idInfo,
    std::array<UserInfo, MAX_USER_CNT>& userInfoList) {
  if (isNewUser(userName, idInfo)) {
    addNewUser(idInfo, userName, userInfoList);
  }
  UserInfo& userInfo = userInfoList.at(getUniformId(userName, idInfo));

  checkIfDebugData(userName);

  if (policy->isSpecialDay(getDayIndex(dayOfWeek)))
    userInfo.specialDayAttended = true;

  userInfo.attendHistory[getDayIndex(dayOfWeek)] += 1;
  userInfo.totalPoints += policy->getDayPoints(getDayIndex(dayOfWeek));
}

void AttendanceSystem::analyzeAllRecord(
    IdInfo& idInfo, std::array<UserInfo, MAX_USER_CNT>& userInfoList) {
  std::ifstream fin{"attendance_weekday_500.txt"};
  for (int i = 0; i < ATTEND_HISTORY_CNT; i++) {
    std::string userName, dayOfWeek;
    fin >> userName >> dayOfWeek;
    analyzeEachRecord(userName, dayOfWeek, idInfo, userInfoList);
  }
}

void AttendanceSystem::analyzeAndPrintUserInfo() {
  IdInfo idInfo;
  std::array<UserInfo, MAX_USER_CNT> userInfoList;

  analyzeAllRecord(idInfo, userInfoList);

  for (int uniformId = 1; uniformId <= idInfo.totalUsers; uniformId++) {
    UserInfo& userInfo = userInfoList.at(uniformId);
    policy->evaluateGrade(userInfoList.at(uniformId));
    userInfo.grade = policy->getUserGrade(userInfo.totalPoints);

    printUserInfo(userInfo);
  }

  printRemovedUser(idInfo, userInfoList);
}

void AttendanceSystem::printUserInfo(const UserInfo& userInfo) {
  std::cout << "NAME : " << userInfo.name << ", ";
  std::cout << "POINT : " << userInfo.totalPoints << ", ";
  std::cout << "GRADE : " << userInfo.grade << "\n";
}

void AttendanceSystem::printRemovedUser(
    const IdInfo& idInfo, std::array<UserInfo, MAX_USER_CNT>& userInfoList) {
  std::cout << "\n";
  std::cout << "Removed player\n";
  std::cout << "==============\n";
  for (int uniformId = 1; uniformId <= idInfo.totalUsers; uniformId++) {
    UserInfo& userInfo = userInfoList.at(uniformId);
    if (policy->isRemovedPlayer(userInfo)) std::cout << userInfo.name << "\n";
  }
}

void AttendanceSystem::checkIfDebugData(const std::string& userName) {
  if (userName == "Daisy") int debug = 1;
}

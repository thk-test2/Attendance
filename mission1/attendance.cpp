#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

constexpr int ATTEND_HISTORY_CNT = 500;
constexpr int MAX_USER_CNT = 100;
constexpr int DAYS_OF_WEEK = 7;

constexpr int NORMALDAY_POINT = 1;
constexpr int WEEKENDS_POINT = 2;
constexpr int TRAININGDAY_POINT = 3;
constexpr int BONUS_POINT = 10;

constexpr int BONUS_ATTEND_DAY_THRESHOLD = 10;
constexpr int GOLD_GRADE_THRESHOLD = 50;
constexpr int SILVER_GRADE_THRESHOLD = 30;

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

bool isNewUser(std::string& userName, IdInfo& idInfo) {
  return idInfo.idTable.count(userName) == 0;
}

void addNewUser(IdInfo& idInfo, std::string& userName,
                std::array<UserInfo, MAX_USER_CNT>& userInfoList) {
  idInfo.idTable.insert({userName, ++(idInfo.totalUsers)});
  userInfoList.at(idInfo.totalUsers).name = userName;
}

int getUniformId(std::string& userName, IdInfo& idInfo) {
  return idInfo.idTable[userName];
}

void checkIfDebugData(std::string& userName) {
  if (userName == "Daisy") int debug = 1;
}

int getDayIndex(std::string& dayOfWeek) {
  if (dayOfWeek == "monday") return DayOfWeek::MONDAY;
  if (dayOfWeek == "tuesday") return DayOfWeek::TUESDAY;
  if (dayOfWeek == "wednesday") return DayOfWeek::WEDNESDAY;
  if (dayOfWeek == "thursday") return DayOfWeek::THURSDAY;
  if (dayOfWeek == "friday") return DayOfWeek::FRIDAY;
  if (dayOfWeek == "saturday") return DayOfWeek::SATURDAY;
  if (dayOfWeek == "sunday") return DayOfWeek::SUNDAY;
}

bool isSpecialDay(int dayIndex) {
  if (dayIndex == DayOfWeek::WEDNESDAY || dayIndex == DayOfWeek::SATURDAY ||
      dayIndex == DayOfWeek::SUNDAY)
    return true;
  return false;
}

int getDayPoints(int dayIndex) {
  if (dayIndex == DayOfWeek::WEDNESDAY)
    return TRAININGDAY_POINT;
  else if ((dayIndex == DayOfWeek::SATURDAY) || (dayIndex == DayOfWeek::SUNDAY))
    return WEEKENDS_POINT;
  return NORMALDAY_POINT;
}

int bonusPointForTrainingDay(UserInfo& userInfo) {
  if (userInfo.attendHistory[DayOfWeek::WEDNESDAY] >=
      BONUS_ATTEND_DAY_THRESHOLD)
    return BONUS_POINT;
  return 0;
}

int bonusPointForWeekends(UserInfo& userInfo) {
  if ((userInfo.attendHistory[DayOfWeek::SATURDAY] +
       userInfo.attendHistory[DayOfWeek::SUNDAY]) >= BONUS_ATTEND_DAY_THRESHOLD)
    return BONUS_POINT;
  return 0;
}

std::string getUserGrade(int totalPoints) {
  if (totalPoints >= GOLD_GRADE_THRESHOLD)
    return "GOLD";
  else if (totalPoints >= SILVER_GRADE_THRESHOLD)
    return "SILVER";
  return "NORMAL";
}

void evaluateGrade(UserInfo& userInfo) {
  userInfo.totalPoints += bonusPointForTrainingDay(userInfo);
  userInfo.totalPoints += bonusPointForWeekends(userInfo);
  userInfo.grade = getUserGrade(userInfo.totalPoints);
}

bool isRemovedPlayer(UserInfo& userInfo) {
  return userInfo.grade == "NORMAL" && userInfo.specialDayAttended == false;
}

void printUserInfo(UserInfo& userInfo) {
  std::cout << "NAME : " << userInfo.name << ", ";
  std::cout << "POINT : " << userInfo.totalPoints << ", ";
  std::cout << "GRADE : " << userInfo.grade << "\n";
}

void printRemovedUser(IdInfo& idInfo,
                      std::array<UserInfo, MAX_USER_CNT>& userInfoList) {
  std::cout << "\n";
  std::cout << "Removed player\n";
  std::cout << "==============\n";
  for (int uniformId = 1; uniformId <= idInfo.totalUsers; uniformId++) {
    UserInfo& userInfo = userInfoList.at(uniformId);
    if (isRemovedPlayer(userInfo)) std::cout << userInfo.name << "\n";
  }
}

void analyzeEachRecord(std::string& userName, std::string& dayOfWeek,
                       IdInfo& idInfo,
                       std::array<UserInfo, MAX_USER_CNT>& userInfoList) {
  if (isNewUser(userName, idInfo)) {
    addNewUser(idInfo, userName, userInfoList);
  }
  UserInfo& userInfo = userInfoList.at(getUniformId(userName, idInfo));

  checkIfDebugData(userName);

  if (isSpecialDay(getDayIndex(dayOfWeek))) userInfo.specialDayAttended = true;

  userInfo.attendHistory[getDayIndex(dayOfWeek)] += 1;
  userInfo.totalPoints += getDayPoints(getDayIndex(dayOfWeek));
}

void analyzeAllRecord(IdInfo& idInfo,
                      std::array<UserInfo, MAX_USER_CNT>& userInfoList) {
  std::ifstream fin{"attendance_weekday_500.txt"};
  for (int i = 0; i < ATTEND_HISTORY_CNT; i++) {
    std::string userName, dayOfWeek;
    fin >> userName >> dayOfWeek;
    analyzeEachRecord(userName, dayOfWeek, idInfo, userInfoList);
  }
}

void analyzeAndPrintUserInfo() {
  IdInfo idInfo;
  std::array<UserInfo, MAX_USER_CNT> userInfoList;

  analyzeAllRecord(idInfo, userInfoList);

  for (int uniformId = 1; uniformId <= idInfo.totalUsers; uniformId++) {
    evaluateGrade(userInfoList.at(uniformId));
    printUserInfo(userInfoList.at(uniformId));
  }

  printRemovedUser(idInfo, userInfoList);
}

int main() { analyzeAndPrintUserInfo(); }
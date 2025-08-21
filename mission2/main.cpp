#include "gmock/gmock.h"

#include "attendance.cpp"

using ::testing::HasSubstr;
using ::testing::internal::CaptureStdout;
using ::testing::internal::GetCapturedStdout;

class AttendTestFixture : public ::testing::Test {
 public:
  UserInfo userInfo;
  IdInfo idInfo;
  
  std::array<UserInfo, MAX_USER_CNT> userInfoList;
  std::unique_ptr<IPolicy> policy;
  std::unique_ptr<AttendanceSystem> system;

  const int NORMALDAY_POINT = 1;
  const int WEEKENDS_POINT = 2;
  const int TRAININGDAY_POINT = 3;
  const int BONUS_POINT = 10;

  const int BONUS_ATTEND_DAY_THRESHOLD = 10;
  const int GOLD_GRADE_THRESHOLD = 50;
  const int SILVER_GRADE_THRESHOLD = 30;

  const std::string testUserName = "testUser";

  const std::string MONDAY_TEXT = "monday";
  const std::string TUESDAY_TEXT = "tuesday";
  const std::string WEDNESDAY_TEXT = "wednesday";
  const std::string THURSDAY_TEXT = "thursday";
  const std::string FRIDAY_TEXT = "friday";
  const std::string SATURDAY_TEXT = "saturday";
  const std::string SUNDAY_TEXT = "sunday";

  const std::string GOLD_TEXT = "GOLD";
  const std::string SILVER_TEXT = "SILVER";
  const std::string NORMAL_TEXT = "NORMAL";

  void SetUp() {
    policy = std::make_unique<CurrentPolicy>();
    system = std::make_unique<AttendanceSystem>(policy.get());
  }
};

TEST_F(AttendTestFixture, isNewUser) {
  bool actual = system->isNewUser(testUserName, idInfo);

  EXPECT_EQ(true, actual);
}

TEST_F(AttendTestFixture, addNewUser) {
  system->addNewUser(idInfo, testUserName, userInfoList);

  EXPECT_EQ(testUserName, userInfoList.at(idInfo.totalUsers).name);
}

TEST_F(AttendTestFixture, getUniformId) {
  system->addNewUser(idInfo, testUserName, userInfoList);
  int actual = system->getUniformId(testUserName, idInfo);

  EXPECT_EQ(1, actual);
}

TEST_F(AttendTestFixture, getDayIndex) {
  EXPECT_EQ(DayOfWeek::MONDAY, system->getDayIndex(MONDAY_TEXT));
  EXPECT_EQ(DayOfWeek::TUESDAY, system->getDayIndex(TUESDAY_TEXT));
  EXPECT_EQ(DayOfWeek::WEDNESDAY, system->getDayIndex(WEDNESDAY_TEXT));
  EXPECT_EQ(DayOfWeek::THURSDAY, system->getDayIndex(THURSDAY_TEXT));
  EXPECT_EQ(DayOfWeek::FRIDAY, system->getDayIndex(FRIDAY_TEXT));
  EXPECT_EQ(DayOfWeek::SATURDAY, system->getDayIndex(SATURDAY_TEXT));
  EXPECT_EQ(DayOfWeek::SUNDAY, system->getDayIndex(SUNDAY_TEXT));
}

TEST_F(AttendTestFixture, isSpecialDay_True) {
  int dayIndex = DayOfWeek::WEDNESDAY;

  bool actual = policy->isSpecialDay(dayIndex);

  EXPECT_EQ(true, actual);
}

TEST_F(AttendTestFixture, isSpecialDay_False) {
  int dayIndex = DayOfWeek::MONDAY;

  bool actual = policy->isSpecialDay(dayIndex);

  EXPECT_EQ(false, actual);
}

TEST_F(AttendTestFixture, getDayPoints) {
  int dayIndex = DayOfWeek::WEDNESDAY;

  int actual = policy->getDayPoints(dayIndex);

  EXPECT_EQ(TRAININGDAY_POINT, actual);
}

TEST_F(AttendTestFixture, bonusPointForTrainingDay) {
  userInfo.attendHistory[DayOfWeek::WEDNESDAY] = 999;

  int actual = policy->bonusPointForTrainingDay(userInfo);

  EXPECT_EQ(BONUS_POINT, actual);
}

TEST_F(AttendTestFixture, bonusPointForWeekends) {
  userInfo.attendHistory[DayOfWeek::SATURDAY] = 999;

  int actual = policy->bonusPointForWeekends(userInfo);

  EXPECT_EQ(BONUS_POINT, actual);
}

TEST_F(AttendTestFixture, getUserGrade_GOLD) {
  int totalPoints = GOLD_GRADE_THRESHOLD;

  std::string actual = policy->getUserGrade(totalPoints);

  EXPECT_EQ(GOLD_TEXT, actual);
}

TEST_F(AttendTestFixture, getUserGrade_SILVER) {
  int totalPoints = SILVER_GRADE_THRESHOLD;

  std::string actual = policy->getUserGrade(totalPoints);

  EXPECT_EQ(SILVER_TEXT, actual);
}

TEST_F(AttendTestFixture, getUserGrade_NORMAL) {
  int totalPoints = 0;

  std::string actual = policy->getUserGrade(totalPoints);

  EXPECT_EQ(NORMAL_TEXT, actual);
}

TEST_F(AttendTestFixture, evaluateGrade) {
  userInfo.totalPoints = 999;

  policy->evaluateGrade(userInfo);

  EXPECT_EQ(GOLD_TEXT, userInfo.grade);
}

TEST_F(AttendTestFixture, isRemovedPlayer) {
  userInfo.grade = NORMAL_TEXT;
  userInfo.specialDayAttended = false;

  bool actual = policy->isRemovedPlayer(userInfo);

  EXPECT_EQ(true, actual);
}

TEST_F(AttendTestFixture, printUserInfo) {
  userInfo.name = testUserName;
  userInfo.totalPoints = 999;
  userInfo.grade = GOLD_TEXT;

  CaptureStdout();
  system->printUserInfo(userInfo);
  std::string actual = GetCapturedStdout();

  EXPECT_THAT(actual, HasSubstr("NAME : test"));
  EXPECT_THAT(actual, HasSubstr("POINT : 999"));
  EXPECT_THAT(actual, HasSubstr("GRADE : GOLD"));
}

TEST_F(AttendTestFixture, printRemovedUser) {
  userInfo.name = testUserName;
  userInfo.totalPoints = 0;
  userInfo.grade = NORMAL_TEXT;

  idInfo.totalUsers = 1;

  userInfoList.at(1) = userInfo;

  CaptureStdout();
  system->printRemovedUser(idInfo, userInfoList);
  std::string actual = GetCapturedStdout();

  EXPECT_THAT(actual, HasSubstr("Removed player"));
  EXPECT_THAT(actual, HasSubstr(testUserName));
}

TEST_F(AttendTestFixture, analyzeEachRecord) {
  system->analyzeEachRecord(testUserName, MONDAY_TEXT, idInfo, userInfoList);

  EXPECT_EQ(1, userInfoList.at(1).attendHistory[system->getDayIndex(MONDAY_TEXT)]);
  EXPECT_EQ(1, userInfoList.at(1).totalPoints);
}

TEST_F(AttendTestFixture, analyzeAllRecord) {
  EXPECT_NO_THROW(system->analyzeAllRecord(idInfo, userInfoList));
}

TEST_F(AttendTestFixture, analyzeAndPrintUserInfo) {
  EXPECT_NO_THROW(system->analyzeAndPrintUserInfo());
}

int main() {
  ::testing::InitGoogleMock();
  return RUN_ALL_TESTS();
}

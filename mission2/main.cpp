#include "gmock/gmock.h"

#include "attendance.cpp"

using ::testing::HasSubstr;
using ::testing::internal::CaptureStdout;
using ::testing::internal::GetCapturedStdout;

class AttendTestFixture : public ::testing::Test {
public:
	UserInfo userInfo;
	IdInfo idInfo;
	std::string testUser{ "test" };
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

	void SetUp() {
		policy = std::make_unique<CurrentPolicy>();
		system = std::make_unique<AttendanceSystem>(policy.get());
		userInfo = UserInfo{};
	}
};

TEST_F(AttendTestFixture, isNewUser) {
	bool actual = system->isNewUser(testUser, idInfo);

	EXPECT_EQ(true, actual);
}

TEST_F(AttendTestFixture, addNewUser) {
	system->addNewUser(idInfo, testUser, userInfoList);

	EXPECT_EQ(testUser, userInfoList.at(idInfo.totalUsers).name);
}

TEST_F(AttendTestFixture, getUniformId) {
	system->addNewUser(idInfo, testUser, userInfoList);
	int actual = system->getUniformId(testUser, idInfo);

	EXPECT_EQ(1, actual);
}

TEST_F(AttendTestFixture, getDayIndex) {
	std::string monday{ "monday" };
	std::string tuesday{ "tuesday" };
	std::string wednesday{ "wednesday" };
	std::string thursday{ "thursday" };
	std::string friday{ "friday" };
	std::string saturday{ "saturday" };
	std::string sunday{ "sunday" };
	EXPECT_EQ(DayOfWeek::MONDAY, system->getDayIndex(monday));
	EXPECT_EQ(DayOfWeek::TUESDAY, system->getDayIndex(tuesday));
	EXPECT_EQ(DayOfWeek::WEDNESDAY, system->getDayIndex(wednesday));
	EXPECT_EQ(DayOfWeek::THURSDAY, system->getDayIndex(thursday));
	EXPECT_EQ(DayOfWeek::FRIDAY, system->getDayIndex(friday));
	EXPECT_EQ(DayOfWeek::SATURDAY, system->getDayIndex(saturday));
	EXPECT_EQ(DayOfWeek::SUNDAY, system->getDayIndex(sunday));
}

TEST_F(AttendTestFixture, isSpecialDay) {
	int dayIndex = DayOfWeek::WEDNESDAY;

	bool actual = policy->isSpecialDay(dayIndex);

	EXPECT_EQ(true, actual);
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

	EXPECT_EQ("GOLD", actual);
}

TEST_F(AttendTestFixture, getUserGrade_SILVER) {
	int totalPoints = SILVER_GRADE_THRESHOLD;

	std::string actual = policy->getUserGrade(totalPoints);

	EXPECT_EQ("SILVER", actual);
}

TEST_F(AttendTestFixture, getUserGrade_NORMAL) {
	int totalPoints = 0;

	std::string actual = policy->getUserGrade(totalPoints);

	EXPECT_EQ("NORMAL", actual);
}

TEST_F(AttendTestFixture, evaluateGrade) {
	userInfo.totalPoints = 999;

	policy->evaluateGrade(userInfo);

	EXPECT_EQ("GOLD", userInfo.grade);
}

TEST_F(AttendTestFixture, isRemovedPlayer) {
	userInfo.grade = "NORMAL";
	userInfo.specialDayAttended = false;

	bool actual = policy->isRemovedPlayer(userInfo);

	EXPECT_EQ(true, actual);
}

TEST_F(AttendTestFixture, printUserInfo) {
	userInfo.name = "test";
	userInfo.totalPoints = 999;
	userInfo.grade = "GOLD";

	CaptureStdout();
	system->printUserInfo(userInfo);
	std::string actual = GetCapturedStdout();

	EXPECT_THAT(actual, HasSubstr("NAME : test"));
	EXPECT_THAT(actual, HasSubstr("POINT : 999"));
	EXPECT_THAT(actual, HasSubstr("GRADE : GOLD"));
}

TEST_F(AttendTestFixture, printRemovedUser) {
	userInfo.name = "test";
	userInfo.totalPoints = 0;
	userInfo.grade = "NORMAL";

	idInfo.totalUsers = 1;

	userInfoList.at(1) = userInfo;

	CaptureStdout();
	system->printRemovedUser(idInfo, userInfoList);
	std::string actual = GetCapturedStdout();

	EXPECT_THAT(actual, HasSubstr("Removed player"));
	EXPECT_THAT(actual, HasSubstr("test"));
}

TEST_F(AttendTestFixture, analyzeEachRecord) {
	std::string test = "test";
	std::string day = "monday";

	system->analyzeEachRecord(test, day, idInfo, userInfoList);

	EXPECT_EQ(1, userInfoList.at(1).attendHistory[system->getDayIndex(day)]);
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

	// analyzeAndPrintUserInfo();
}

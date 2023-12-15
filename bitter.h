// Copyright 2022-2023 WildFox Studio - Kirichenko Stanislav
// No warranty implied. Use it at your own risk

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// USAGE with macros

// Test class definition
// example file: myclass.test.cpp
//
//  TEST_DEFINE_CLASS(MyTestClass) // Class name must be unique
//          int RandomNumberAboveZero();
//          int MyVariable{};
//  TEST_END_CLASS(MyTestClass)
//
//  void MyTestClass::Define()
//  {
//      //Define our test cases
//      TestCase("MyVariable should not be zero", [this]() {
//          MyVariable = RandomNumberAboveZero();
//          TEST_NEQUAL(0, MyVariable);
//      }
//  }

// example file: main.test.cpp
//
//  #include "bitter.h"
//  int main(int argc, char* argv[])
//  {
//      RUN_ALL_TESTS(argc, argv);
//  };

// USAGE WITHOUT MACROS

// Test class definition
// example file: myclass.test.cpp
//  class MyTestClass final : public bitterAutomatedTestInstance {
//     public:
//          virtual void Define() override;
//     protected:
//          int RandomNumberAboveZero();
//          int MyVariable{};
//  };
//
//  //Register class statically or add it manually in the main.test.cpp but not both
//  static bitterTestInserter<MyTestClass> RegisterMyTestClass("MyTestClass");
//
//  void MyTestClass::Define()
//  {
//      //Define our test cases
//      TestCase("MyVariable should not be zero", [this]() {
//          MyVariable = RandomNumberAboveZero();
//          TEST_NEQUAL(0, MyVariable);
//      }
//  }

// example file: main.test.cpp
//
//  #include "bitter.h"
//
//  int main(int argc, char* argv[]) {
//     bitterAutomationTester engineTester;
//     engineTester.AddTest<MyTestClass>("MyTestClass");
//     engineTester.AddTest<MyOtherTestClass>("MyOtherTestClass");
//
//     return engineTester.RunAllTests();
// };

// When launching the executable you can pass a filename that will be used a log (the path must exist)
// ~ test.exe testResult.txt

#pragma once

#include <algorithm>
#include <cassert>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#define TEXT_RED "\033[31m"
#define TEXT_GREEN "\033[32m"
#define TEXT_WHITE "\033[37m"
#define ENDLINE static_cast<char>(0x0a)

namespace bitter
{
	inline bool
		__floatsAlmostSame(float a, float b, float epsilon = std::numeric_limits<float>().epsilon())
	{
		return (std::fabs(a - b) < epsilon);
	}

	inline bool
		__doublesAlmostSame(double a, double b, double epsilon = std::numeric_limits<double>().epsilon())
	{
		return (std::abs(a - b) < epsilon);
	}

	/*Defines the current status of a given test case*/
	enum class ETestStatus
	{
		NOT_TESTED,
		PASSED,
		FAILED
	};

	/*Wraps a functions the will execute a test case*/
	struct DTestCase
	{
	public:
		DTestCase(const std::string& name, std::function<void(void)> testFunction) : Name(name.c_str()), Func(testFunction) {};
		inline void               DoWork() const { Func(); };
		std::string               Name;
		std::function<void(void)> Func;
	};

	/*This is the class responsible of defining a group of test cases*/
	class AutomatedTestInstance
	{
	public:
		AutomatedTestInstance() = default;
		virtual ~AutomatedTestInstance() = default;

		/*Resets it's internal state*/
		inline void ResetFlags()
		{
			_failed = false;
			_currentRunningTest = -1;
		};

		/*Overidde this function to define the test cases*/
		virtual void Define() = 0;

		/*If expression == false it will make the test fail*/
		inline bool TestTrue(bool expression)
		{
			if (!expression)
			{
				_failed = true;
			}
			return expression;
		};

		/*If expression == true it will make the test fail*/
		inline bool TestFalse(bool expression)
		{
			if (expression)
			{
				_failed = true;
			}
			return !expression;
		};

		/*Compare two values and return true if they are equal, if not the test will fail*/
		bool TestEqual(char value, char expected)
		{
			if (!(value == expected))
			{
				_failed = true;
				return false;
			}
			return true;
		};

		bool TestEqual(unsigned char value, unsigned char expected)
		{
			if (!(value == expected))
			{
				_failed = true;
				return false;
			}
			return true;
		};

		/*Compare two values and return true if they are equal, if not the test will fail*/
		bool TestEqual(int value, int expected)
		{
			if (!(value == expected))
			{
				_failed = true;
				return false;
			}
			return true;
		};

		/*Compare two values and return true if they are equal, if not the test will fail*/
		bool TestEqual(unsigned int value, unsigned int expected)
		{
			if (!(value == expected))
			{
				_failed = true;
				return false;
			}
			return true;
		};

		/*Compare two values and return true if they are equal, if not the test will fail*/
		bool TestEqual(float value, float expected)
		{
			if (!__floatsAlmostSame(value, expected))
			{
				_failed = true;
				return false;
			}
			return true;
		};

		/*Compare two values and return true if they are equal, if not the test will fail*/
		bool TestEqual(double value, double expected)
		{
			if (!__doublesAlmostSame(value, expected))
			{
				_failed = true;
				return false;
			}
			return true;
		};

		/*Return a vector of test names*/
		inline std::vector<std::string> GetTestNames() const
		{
			std::vector<std::string> names;
			names.reserve(_tests.size());
			std::transform(_tests.begin(), _tests.end(), std::back_inserter(names), [](const DTestCase& test) { return test.Name; });
			return names;
		};

		/*Will run a particular test case by it's name*/
		inline bool RunTest(const std::string& name)
		{
			ResetFlags();
			auto found = std::find_if(_tests.begin(), _tests.end(), [name](const DTestCase& test) { return test.Name == name; });
			assert(found != _tests.end());
			_currentRunningTest = static_cast<signed int>(std::distance(_tests.begin(), found));
			try
			{
				found->DoWork();
				_testStatus[_currentRunningTest] = _failed ? ETestStatus::FAILED : ETestStatus::PASSED;
			}
			catch (...)
			{
				_testStatus[_currentRunningTest] = ETestStatus::FAILED;
			}
			return !_failed;
		}

		/*Run all tests, return true if they all passed, false otherwise*/
		inline bool RunAll()
		{
			unsigned int passed{};
			for (int i = 0; i < _tests.size(); i++)
			{
				ResetFlags();
				const auto& test = _tests[i];
				try
				{
					passed += static_cast<unsigned int>(RunTest(test.Name));
				}
				catch (...)
				{
				}
			}

			return (passed == _tests.size());
		}

		/*Get the status of a particular test by name*/
		inline ETestStatus GetResult(const std::string& name) const
		{
			for (int i = 0; i < _tests.size(); i++)
			{
				const auto& test = _tests[i];
				if (test.Name == name)
				{
					return _testStatus[i];
				}
			}
			assert(0);//Test name does not exists
			return ETestStatus::NOT_TESTED;
		}

		/*Get a vector of status for all the tests*/
		inline std::vector<ETestStatus> GetResults() const { return _testStatus; }

		/*Used to define a test case*/
		inline void TestCase(const std::string& name, std::function<void(void)> testFunc)
		{
			// check that does not exists with same name
			assert(std::find_if(_tests.begin(), _tests.end(), [name](const DTestCase& test) { return test.Name == name; }) == _tests.end());
			assert(_tests.size() < std::numeric_limits<signed int>().max());
			try
			{
				_tests.push_back(DTestCase(name, std::move(testFunc)));
				_testStatus.push_back(ETestStatus::NOT_TESTED);
			}
			catch (...)
			{
				assert(0); // Failed to allocate test case
			}
		};

		/*Returns the index of the current running test. Returns -1 if no tests are running*/
		inline signed int GetCurrentRunningTest() const { return _currentRunningTest; };

		inline std::string        GetLog() const { return _log.str(); };
		inline void               ResetLog() { _log.clear(); };
		inline std::stringstream& OutLog() { return _log; };

	private:
		std::vector<DTestCase>   _tests;
		std::vector<ETestStatus> _testStatus;
		bool                     _failed{};
		std::stringstream        _log;
		signed int               _currentRunningTest = -1;
	};

	class AutomationTester
	{
	public:
		AutomationTester() = default;

		// Define it as a singleton
		inline static AutomationTester& GetInstance()
		{
			static AutomationTester tester;
			return tester;
		};

		template<class T>
		void AddTest(const std::string& testName)
		{
			_tests[testName] = []() -> AutomatedTestInstance* { return new T; };
		};

		bool RunAllTests(int argc = 0, char* argv[] = nullptr)
		{
			SetupOutstream(argc, argv);

			unsigned int testPassed{};
			for (const auto& newTest : _tests)
			{

				GetOutstream() << TEXT_WHITE << ENDLINE << "Begin testing:" << newTest.first << ENDLINE;
				GetOutstream().flush();

				AutomatedTestInstance* testInstance = newTest.second();
				testInstance->Define();
				unsigned int subTestNumPassed = 0;
				for (const auto& n : testInstance->GetTestNames())
				{
					GetOutstream() << TEXT_WHITE << "Running:" << n << ENDLINE;
					GetOutstream().flush();
					// Run the test
					const bool result = testInstance->RunTest(n);
					OutResult(result);
					GetOutstream() << ENDLINE;
					GetOutstream().flush();
					// Increment counter
					subTestNumPassed += static_cast<unsigned int>(result);
				}
				GetOutstream() << TEXT_RED << testInstance->GetLog() << ENDLINE;
				GetOutstream() << TEXT_GREEN << "Result completed tests [" << subTestNumPassed << "/" << testInstance->GetTestNames().size() << "]" << ENDLINE;

				GetOutstream() << TEXT_WHITE << newTest.first << " Completed with result" << ENDLINE;
				OutResult((subTestNumPassed == testInstance->GetTestNames().size()));
				testPassed += (static_cast<unsigned int>(subTestNumPassed == testInstance->GetTestNames().size()));

				GetOutstream() << ENDLINE << ENDLINE;
				GetOutstream().flush();
				delete testInstance;
			}

			GetOutstream() << TEXT_WHITE << "Testing ended with result" << ENDLINE;
			OutResult(testPassed == _tests.size());
			GetOutstream() << ENDLINE;

			GetOutstream() << TEXT_WHITE;
			EndOutputStream();

			return (testPassed == _tests.size());
		};

		inline std::ostream& GetOutstream()
		{
			if (_outstream.is_open())
			{
				return _outstream;
			}

			return std::cerr;
		};

	private:
		static constexpr unsigned int _resultOffset = 60;
		std::ofstream                 _outstream;

		inline void SetupOutstream(int argc, char* argv[])
		{
			if (argc > 1)
			{
				_outstream.open(argv[1]);
				if (_outstream.is_open())
				{
					std::cerr << "Writing to file:" << argv[1];
				}
				else
				{
					std::cerr << "Could not create log with filename:" << argv[1];
				}
			}
		};

		inline void EndOutputStream()
		{
			GetOutstream().flush();
			if (_outstream.is_open())
			{
				_outstream.close();
			}
		};

		inline void OutSuccess() { GetOutstream() << std::setfill('-') << std::setw(_resultOffset) << "[" << TEXT_GREEN << "PASSED" << TEXT_WHITE << "]"; };
		inline void OutFailure() { GetOutstream() << std::setfill('-') << std::setw(_resultOffset) << "[" << TEXT_RED << "FAILED" << TEXT_WHITE << "]"; };

		inline void OutResult(bool result)
		{
			if (result)
			{
				GetOutstream() << std::setfill('-') << std::setw(_resultOffset) << "[" << TEXT_GREEN << "PASSED" << TEXT_WHITE << "]";
			}
			else
			{
				GetOutstream() << std::setfill('-') << std::setw(_resultOffset) << "[" << TEXT_RED << "FAILED" << TEXT_WHITE << "]";
			}
		};

	private:
		std::map<std::string, std::function<AutomatedTestInstance* (void)>> _tests;
	};

	template<class T>
	class TestInserter
	{
	public:
		TestInserter(const std::string& className) { AutomationTester::GetInstance().AddTest<T>(className); };
	};

} // namespace Fox

#define ADD_TEST(testClass) static bitter::TestInserter<testClass> __testInserted##testClass(#testClass);

#define TEST_DEFINE_CLASS(className) \
    class className final : public bitter::AutomatedTestInstance \
    { \
      public: \
        void Define() override;

#define TEST_END_CLASS(className) \
    } \
    ; \
    static bitter::TestInserter<className> __testInserted##className(#className);

#define TEST_TRUE_OR_QUIT(expression) \
    { \
        if (!TestTrue((expression))) \
            { \
                std::cerr << "In:" << GetTestNames()[GetCurrentRunningTest()] << "[line " << __LINE__ << "]" \
                          << " TEST_TRUE_OR_QUIT(" << #expression << ")" \
                          << " was expected to be true but it was false" << ENDLINE; \
                return; \
            } \
    }

#define TEST_TRUE(expression) \
    { \
        if (!TestTrue((expression))) \
            { \
                std::cerr << "In:" << GetTestNames()[GetCurrentRunningTest()] << "[line " << __LINE__ << "]" \
                          << " TEST_TRUE(" << #expression << ")" \
                          << " was expected to be true but it was false" << ENDLINE; \
            } \
    }

#define TEST_FALSE(expression) \
    { \
        if (TestFalse((expression))) \
            { \
                std::cerr << "In:" << GetTestNames()[GetCurrentRunningTest()] << "[line " << __LINE__ << "]" \
                          << " TEST_FALSE(" << #expression << ")" \
                          << " was expected to be true but it was false" << ENDLINE; \
            } \
    }

#define TEST_EQUAL(a, b) \
    { \
		if (!TestEqual((a), (b)))\
			{\
					std::cerr << "In:" << GetTestNames()[GetCurrentRunningTest()] << "[line " << __LINE__ << "]" \
							<< " TEST_EQUAL(" << #a << "," << #b << ")" \
							<< " was expected to be equal but they are not" << ENDLINE; \
			} \
    }

#define TEST_NEQUAL(a, b) \
    { \
		if (TestEqual((a), (b)))\
			{\
					std::cerr << "In:" << GetTestNames()[GetCurrentRunningTest()] << "[line " << __LINE__ << "]" \
							<< " TEST_NEQUAL(" << #a << "," << #b << ")" \
							<< " was expected to be different but they are the same" << ENDLINE; \
			} \
    }

// Returns 0  when all tests succed or 1 when at least one test has failed
#define RUN_ALL_TESTS(argc, argv) return !bitterAutomationTester::GetInstance().RunAllTests(argc, argv);
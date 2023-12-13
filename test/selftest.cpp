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

#include "../foxtest.h"

#include <cassert>
#include <limits>
#include <algorithm>



void MultipleTestsShouldExecute()
{
	static signed int counterA{};
	static signed int counterB{};

	class TestA final : public Fox::AutomatedTestInstance {
	public:
		virtual void Define() override {
			//Define our test cases
			TestCase("Should increase counterA", []() {
				counterA++;
				});
		}
	};

	class TestB final : public Fox::AutomatedTestInstance {
	public:
		virtual void Define() override {
			//Define our test cases
			TestCase("Should increase counterB", []() {
				counterB++;
				});
		}
	};

	Fox::AutomationTester tester;
	tester.AddTest<TestA>("TestA");
	tester.AddTest<TestB>("TestB");

	counterA = 0;
	counterB = 0;

	assert(tester.RunAllTests() == true);
	assert(counterA == 1);
	assert(counterB == 1);
};

void MultipleTestsShouldAllSucced()
{
	class TestA final : public Fox::AutomatedTestInstance {
	public:
		virtual void Define() override {
			//Define our test cases
			TestCase("Should pass", [this]() {
				TEST_TRUE(true);
			TEST_FALSE(false);
			TEST_EQUAL(0x0, 0x0);
			TEST_EQUAL(1, 1);
			TEST_EQUAL(1.f, 1.f);
			TEST_EQUAL(10., 10.);
				});
		}
	};

	Fox::AutomationTester tester;
	tester.AddTest<TestA>("A");
	tester.AddTest<TestA>("B");
	tester.AddTest<TestA>("C");

	assert(tester.RunAllTests() == true);
};

void MultipleTestsShouldSuccedButOneShouldFail()
{
	class TestA final : public Fox::AutomatedTestInstance {
	public:
		virtual void Define() override {
			//Define our test cases
			TestCase("Should pass", [this]() {
				TEST_TRUE(true);
				});
		}
	};

	class TestFailing final : public Fox::AutomatedTestInstance {
	public:
		virtual void Define() override {
			//Define our test cases
			TestCase("Should fail", [this]() {
				TEST_TRUE(false);
				});
		}
	};

	Fox::AutomationTester tester;
	tester.AddTest<TestA>("A");
	tester.AddTest<TestFailing>("TestFailing");
	tester.AddTest<TestA>("C");

	assert(tester.RunAllTests() == false);
};

void InstanceShouldReturnCorrectValues()
{
	class Instance final : public Fox::AutomatedTestInstance {
	public:
		virtual void Define() override {}
	};
	Instance inst;

	assert(inst.TestEqual((char)-100, (char)-100) == true);
	assert(inst.TestEqual((char)-100, (char)-101) == false);

	assert(inst.TestEqual((unsigned char)100, (unsigned char)100) == true);
	assert(inst.TestEqual((unsigned char)100, (unsigned char)101) == false);

	assert(inst.TestEqual((int)-100, (int)-100) == true);
	assert(inst.TestEqual((int)-100, (int)-101) == false);

	assert(inst.TestEqual((unsigned int)100, (unsigned int)100) == true);
	assert(inst.TestEqual((unsigned int)100, (unsigned int)101) == false);

	assert(inst.TestEqual(1.f, 1.f) == true);
	assert(inst.TestEqual(1.f, 1.f + std::numeric_limits<float>().epsilon()) == false);

	assert(inst.TestEqual(1., 1.) == true);
	assert(inst.TestEqual(1., 1. + std::numeric_limits<double>().epsilon()) == false);
};

void InstanceShouldListAllTestNames()
{
	class Instance final : public Fox::AutomatedTestInstance {
	public:
		virtual void Define() override {
			TestCase("A", []() {});
			TestCase("B", []() {});
			TestCase("C", []() {});
		}
	};
	Instance inst;
	inst.Define();

	const auto testNames = inst.GetTestNames();
	assert(testNames.size() == 3);
	assert(std::find(testNames.begin(), testNames.end(), "A") != testNames.end());
	assert(std::find(testNames.begin(), testNames.end(), "B") != testNames.end());
	assert(std::find(testNames.begin(), testNames.end(), "C") != testNames.end());
};

void InstanceShouldRunTestByNames()
{
	class Instance final : public Fox::AutomatedTestInstance {
	public:
		virtual void Define() override {
			TestCase("A", [this]() {TEST_TRUE(true); });
			TestCase("B", [this]() {TEST_TRUE(false); });
			TestCase("C", [this]() {TEST_TRUE(true); });
		}
	};
	Instance inst;
	inst.Define();

	assert(inst.RunTest("A") == true);
	assert(inst.RunTest("B") == false);
	assert(inst.RunTest("C") == true);
};


int main(int argc, char* argv[])
{
	MultipleTestsShouldExecute();
	MultipleTestsShouldAllSucced();
	MultipleTestsShouldSuccedButOneShouldFail();
	InstanceShouldReturnCorrectValues();
	InstanceShouldListAllTestNames();
	InstanceShouldRunTestByNames();

	return 0;
};
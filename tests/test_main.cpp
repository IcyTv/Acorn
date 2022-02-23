#include <Acorn/core/Log.h>
#include <gtest/gtest.h>

int main(int argc, char** argv)
{
	// TODO: We need a better way to do logging: What I want is to only output to the console when the test fails and then
	// split those logs based on the test.
	Acorn::Log::Init();
	Acorn::Log::GetCoreLogger()->set_level(spdlog::level::warn);
	Acorn::Log::GetClientLogger()->set_level(spdlog::level::warn);
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
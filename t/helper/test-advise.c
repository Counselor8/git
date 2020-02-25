#include "test-tool.h"
#include "cache.h"
#include "advice.h"

int cmd__advise_if_enabled(int argc, const char **argv)
{
	if (!argv[1])
	die("usage: %s <advice>", argv[0]);

	setup_git_directory();

	/*
	  Any advice type can be used for testing, but NESTED_TAG was selected
	  here and in t0018 where this command is being executed.
	 */
	advise_if_enabled(NESTED_TAG, argv[1]);

	return 0;
}

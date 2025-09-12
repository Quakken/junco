/**
 * @note For these tests to work, the directory of the output binary must be set
 * up properly. A test has been included to format the directory automatically,
 * but to speed up future tests it can be disabled.
 */

#include "junco/fs.hpp"  // junco::File, junco::Directory, junco::FileSystem
#include <filesystem>    // std::filesystem
#include <gtest/gtest.h> // Testing
#include <thread>        // std::thread

// Whether to setup the project's filesystem before running tests.
#define DO_SETUP true

// Path to the binary output directory.
const auto root_path = std::filesystem::current_path();
junco::FileSystem testing_fs{root_path};

void expect_file_contents(junco::File &file, const char *expected) {
  auto f_contents = file.get_contents();
  EXPECT_STREQ(expected, f_contents.c_str());
}

// Messages for all of the test files
constexpr const char *test_contents[] = {
    "This is a message read from the first test!",
    "This is a message written to the second test!"};

/**
 * Initializes the test's structure.
 * @note Because this test relies on the filesystem itself, if certain parts of
 * the filesystem are not working as intended, this could fail and leave other
 * tests in an unstable state.
 */
#if DO_SETUP
TEST(FileTesting, Setup) {
  auto &test_dir = testing_fs.open_directory("fs_test");
  auto &test1 = test_dir.open_file("test1.txt");
  test1.set_contents(test_contents[0]);
}
#endif

/**
 * Tests file input and ifstream-like behavior.
 */
TEST(FileTesting, FileInput) {
  // Should not throw exception
  auto &file = testing_fs.open_file("fs_test/test1.txt");
  // get_contents()
  {
    auto contents = file.get_contents();
    EXPECT_STREQ(test_contents[0], contents.c_str());
  }
  // read()
  {
    auto size = file.get_size();
    auto contents = file.read(0, size);
    EXPECT_STREQ(test_contents[0], contents.c_str());
  }
}

/**
 * Tests file output and ofstream-like behavior.
 */
TEST(FileTesting, FileOutput) {
  auto &file = testing_fs.open_file("fs_test/test2.txt");
  // set_contents()
  {
    file.set_contents(test_contents[1]);
    expect_file_contents(file, test_contents[1]);
    // Erase file contents
    file.clear();
  }
  // write()
  {
    file.write(test_contents[1], 0);
    expect_file_contents(file, test_contents[1]);
    file.clear();
  }
  // append()
  {
    file.append(test_contents[1]);
    expect_file_contents(file, test_contents[1]);
    file.clear();
  }
}
/**
 * Tests File's filesystem operations, like renaming, path resolution, etc.
 */
TEST(FileTesting, FileOps) {
  auto &file = testing_fs.open_file("fs_test/test3.txt");

  auto name = file.get_name();
  EXPECT_STREQ("test3.txt", name.c_str());

  auto extension = file.get_extension();
  EXPECT_STREQ(".txt", extension.c_str());

  auto stem = file.get_stem();
  EXPECT_STREQ("test3", stem.c_str());

  auto path = file.get_path();
  auto path_str = path.string();
  auto expected_path = root_path / "fs_test" / "test3.txt";
  auto expected_path_str = expected_path.string();
  EXPECT_STREQ(expected_path_str.c_str(), path_str.c_str());

  auto size = file.get_size();
  EXPECT_EQ(0, size);
}

/**
 * Tests concurrent access of files.
 */
TEST(FileTesting, ConcurrentAccess) {}

/**
 * Tests file access from directories.
 */
TEST(DirectoryTesting, GetFiles) {}

/**
 * Tests directory access from directories.
 */
TEST(DirectoryTesting, GetDirectories) {}

TEST(DirectoryTesting, Exceptions) {}

/**
 * Tests iterator protocol for directories.
 */
TEST(DirectoryTesting, Iterators) {}

/**
 * Tests concurrent access of directories.
 */
TEST(DirectoryTesting, ConcurrentAccess) {}
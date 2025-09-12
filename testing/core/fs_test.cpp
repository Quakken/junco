/**
 * @note For these tests to work, the directory of the output binary must be set
 * up properly. A test has been included to format the directory automatically,
 * but to speed up future tests it can be disabled.
 */

#include "junco/fs.hpp"  // junco::File, junco::Directory, junco::FileSystem
#include <chrono>        // std::chrono_literals
#include <filesystem>    // std::filesystem
#include <gtest/gtest.h> // Testing
#include <thread>        // std::thread

// Whether to setup the project's filesystem before running tests.
#define DO_SETUP false

using namespace std::chrono_literals;
void wait_for_work() { std::this_thread::sleep_for(10ms); }

// Path to the binary output directory.
const auto root_path = std::filesystem::current_path();

junco::FileSystem testing_fs{root_path};

void expect_file_contents(const junco::File &file, const char *expected) {
  auto f_contents = file.get_contents();
  auto f_contents_cstr = f_contents.c_str();
  EXPECT_STREQ(expected, f_contents_cstr);
}

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
  test1.set_contents("These are the contents of the first file!");
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
    auto contents_cstr = contents.c_str();
    EXPECT_STREQ("These are the contents of the first test file!",
                 contents_cstr);
  }
  // read()
  {
    auto size = file.get_size();
    auto contents = file.read(0, size);
    auto contents_cstr = contents.c_str();
    EXPECT_STREQ("These are the contents of the first test file!",
                 contents_cstr);
  }
}

/**
 * Tests file output and ofstream-like behavior.
 */
TEST(FileTesting, FileOutput) {
  auto &file = testing_fs.open_file("fs_test/test2.txt");
  // set_contents()
  {
    file.set_contents("These are the contents of the second file!");
    expect_file_contents(file, "These are the contents of the second file!");
    // Erase file contents
    file.set_contents("");
  }
  // write()
  {
    file.write("These are the contents of the second file!", 0);
    expect_file_contents(file, "These are the contents of the second file!");
    file.set_contents("");
  }
  // append()
  {
    file.append("These are the contents of the second file!");
    expect_file_contents(file, "These are the contents of the second file!");
  }
}
/**
 * Tests File's filesystem operations, like renaming, path resolution, etc.
 */
TEST(FileTesting, FileOps) {
  auto &file = testing_fs.open_file("fs_test/test3.txt");
  auto name = file.get_name();
  auto name_cstr = name.c_str();
  EXPECT_STREQ("test3.txt", name_cstr);
  auto extension = file.get_extension();
  auto extension_cstr = extension.c_str();
  EXPECT_STREQ("txt", extension_cstr);
  auto stem = file.get_stem();
  auto stem_cstr = stem.c_str();
  EXPECT_STREQ("test3", stem_cstr);
  // auto path = file.get_path();
  // TODO: Path should be relative to filesystem root dir
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
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
#define DO_SETUP false

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
    "This is a message written to the second test!",
    "", /*unused*/
    "These are the contents of the concurrent test!",
    "", /*unused*/
    "This is the message written to the read/write test!\n"};

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

  auto &test4 = test_dir.open_file("test4.txt");
  test4.set_contents((test_contents[3]));
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

TEST(FileTesting, ConcurrentRead) {
  std::string strings[10]{""};
  auto threads = std::vector<std::thread>{};
  auto read_into_string = [](std::string &str) {
    auto &file = testing_fs.open_file("fs_test/test4.txt");
    str = file.get_contents();
  };

  for (auto &string : strings)
    threads.emplace_back(std::thread(read_into_string, std::ref(string)));

  for (auto &thread : threads)
    thread.join();

  // Make sure all strings were read to
  for (const auto &string : strings) {
    EXPECT_STREQ(test_contents[3], string.c_str());
  }
}

TEST(FileTesting, ConcurrentWrite) {
  auto &file = testing_fs.open_file("fs_test/test5.txt");
  file.clear();

  auto messages = {"Never ", "gonna ",  "give ", "you ",    "up, ",   "never ",
                   "gonna ", "let ",    "you ",  "down. ",  "Never ", "gonna ",
                   "run ",   "around ", "and ",  "desert ", "you "};
  auto threads = std::vector<std::thread>{};
  auto write_to_file = [&file](const char *message) { file.append(message); };

  for (const auto &message : messages)
    threads.emplace_back(std::thread(write_to_file, message));

  for (auto &thread : threads)
    thread.join();

  // Make sure all messages sent correctly (all present, not corrupted)
  auto contents = file.get_contents();
  for (const auto &message : messages) {
    EXPECT_NE(contents.find(message), std::string::npos);
  }
}

TEST(FileTesting, ConcurrentReadWrite) {
  auto &file = testing_fs.open_file("fs_test/test6.txt");
  file.clear();

  constexpr int num_threads = 10;
  auto message = test_contents[5];
  auto write_threads = std::vector<std::thread>{};
  auto read_threads = std::vector<std::thread>{};
  // Results of the read operations
  auto results = std::vector<std::string>(num_threads);

  auto write_to_file = [&file, &message]() { file.append(message); };
  for (int i = 0; i < num_threads; ++i)
    write_threads.emplace_back(write_to_file);

  auto read_from_file = [&file](std::string &out) {
    out = file.get_contents();
  };
  for (auto &result : results)
    read_threads.emplace_back(std::thread(read_from_file, std::ref(result)));

  for (auto &writer : write_threads)
    writer.join();
  for (auto &reader : read_threads)
    reader.join();

  // Make sure that the message was written once for each thread
  auto contents = file.get_contents();
  auto count = 0;
  auto pos = size_t{0};
  while ((pos = contents.find(message, pos)) != std::string::npos) {
    ++count;
    pos += std::string(message).size();
  }
  EXPECT_EQ(count, num_threads);

  // Validate: all read threads saw at least one full message
  for (const auto &result : results) {
    EXPECT_NE(result.find(message), std::string::npos);
  }
}

/**
 * Tests file access from directories.
 */
TEST(DirectoryTesting, GetFiles) {
  // Path obfuscated to test indirect path resolution
  auto &dir = testing_fs.open_directory("./fs_test/../fs_test/");
  auto files = {"test1.txt", "test2.txt", "test3.txt",
                "test4.txt", "test5.txt", "test6.txt"};
  for (const auto &file : files)
    EXPECT_NO_THROW(dir.get_file(file));
}

/**
 * Tests directory access from directories.
 */
TEST(DirectoryTesting, GetDirectories) {
  auto &test_dir = testing_fs.open_directory("fs_test/dir_test");

  // Create a number of subdirectories
  auto dir_names = {"I", "am", "the", "storm", "that", "is", "approaching"};
  for (const auto &name : dir_names) {
    test_dir.open_directory(name);
  }
  // Make sure the directories were created
  for (const auto &name : dir_names) {
    EXPECT_NO_THROW(test_dir.get_directory(name));
  }
  // Get info about the directories
  for (const auto &name : dir_names) {
    auto &dir = test_dir.get_directory(name);
    EXPECT_STREQ(name, dir.get_name().c_str());
    EXPECT_TRUE(std::filesystem::exists(dir.get_path()));
    EXPECT_TRUE(std::filesystem::is_directory(dir.get_path()));
  }
}

TEST(DirectoryTesting, Exceptions) {
  auto &dir = testing_fs.open_directory("fs_test/empty_dir");

  auto bad_directories = {"I",       "was",      "born", "in",  "an",
                          "estuary", "destined", "to",   "the", "sea"};
  for (const auto &bad : bad_directories)
    EXPECT_THROW(dir.get_directory(bad), junco::EntryNotFoundException);

  auto bad_files = {"I",    "was", "surrounded", "by",  "pearls",
                    "that", "I",   "could",      "not", "eat"};
  // and diamonds that I could not drink.

  for (const auto &bad : bad_files)
    EXPECT_THROW(dir.get_file(bad), junco::EntryNotFoundException);
}

/**
 * Tests concurrent access of directories.
 */
TEST(DirectoryTesting, ConcurrentAccess) {
  auto dir_names = {"I", "am", "the", "storm", "that", "is", "approaching"};
  auto threads = std::vector<std::thread>{};

  auto &dir = testing_fs.open_directory("fs_test/dir_test");

  // Test cache safety
  auto get_directory = [&dir](const std::string &name) {
    dir.get_directory(name);
  };
  for (const auto &name : dir_names) {
    threads.emplace_back(std::thread(get_directory, name));
  }
  // Cache should still be valid at this point (this should not throw an
  // exception)
  for (const auto &name : dir_names) {
    dir.get_directory(name);
  }
}
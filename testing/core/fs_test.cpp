#include "junco/fs.hpp"
#include <gtest/gtest.h>

junco::FileSystem testing_fs{"./fs_test/"};

void expect_file_contents(const junco::File &file, const char *expected) {
  auto f_contents = file.get_contents();
  auto f_contents_cstr = f_contents.c_str();
  EXPECT_STREQ(expected, f_contents_cstr);
}

/**
 * Tests file input and ifstream-like behavior.
 */
TEST(FileTesting, FileInput) {
  // Should not throw exception
  auto &file = testing_fs.open_file("test1.txt");
  // get_contents()
  {
    auto contents = file.get_contents();
    auto contents_cstr = contents.c_str();
    EXPECT_STREQ("These are the contents of the first test file!",
                 contents_cstr);
  }
  // Extraction operator
  {
    auto contents = std::string{};
    auto token = std::string{};
    file >> token;
    while (!token.empty()) {
      contents += token;
      file >> token;
    }
    auto contents_cstr = contents.c_str();
    EXPECT_STREQ("Thesearethecontentsofthefirsttestfile!", contents_cstr);
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
  auto &file = testing_fs.open_file("test2.txt");
  // set_contents()
  {
    file.set_contents("These are the contents of the second file!");
    expect_file_contents(file, "These are the contents of the second file!");
    // Erase file contents
    file.set_contents("");
  }
  // Insertion operator
  {
    auto words = {"These ", "are ", "some ", "words!"};
    for (const auto &word : words) {
      file << word;
    }
    expect_file_contents(file, "These are some words!");
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
  auto &file = testing_fs.open_file("test3.txt");
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
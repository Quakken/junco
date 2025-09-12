/**
 * @file junco/fs.hpp
 *
 * Declares junco's filesystem utilities and objects, including Files and
 * Directories.
 */

#pragma once

#include <filesystem> // std::filesystem
#include <fstream>    // std::fstream, std::ifstream
#include <shared_mutex>
#include <string>        // std::string
#include <unordered_map> // std::unordered_map

#include "junco/exception.hpp" // junco::Exception

namespace junco {

class DirectoryEntry;
class FileSystem;
class Directory;

class FileSystemException : public Exception {
public:
  FileSystemException(const char *what = "FileSystem exception") noexcept
      : Exception(what) {}
};

class EntryNotFoundException : public FileSystemException {
public:
  EntryNotFoundException(const char *what = "EntryNotFound exception") noexcept
      : FileSystemException(what) {}
};

class InvalidPathException : public FileSystemException {
public:
  InvalidPathException(const char *what = "InvalidPath exception") noexcept
      : FileSystemException(what) {}
};

/**
 * Wraps an std::fstream object for thread-safe file I/O.
 */
class File final {
public:
  // Files in junco are always passed by reference, never by value.
  File(const File &) = delete;
  File &operator=(const File &) = delete;

  /**
   * Retrieves the entire contents of the file.
   */
  std::string get_contents() noexcept;
  /**
   * Overwrites the entire contents of the file.
   */
  void set_contents(const std::string &new_contents) noexcept;

  void append(const std::string &data) noexcept;

  /**
   * Writes (or overwrites) file data at the given position.
   */
  void write(const std::string &data, const std::size_t &pos) noexcept;
  /**
   * Reads a number of bytes from the file, starting at the given position.
   */
  std::string read(const std::size_t &pos, const std::size_t &count) noexcept;

  void clear() noexcept;

  void set_name(const std::string &new_name) noexcept;
  std::string get_name() const noexcept;
  std::string get_extension() const noexcept;
  std::string get_stem() const noexcept;
  std::filesystem::path get_path() const noexcept;
  std::size_t get_size() const noexcept;

private:
  friend Directory;
  explicit File(const std::filesystem::path &path) noexcept;

  /*
   * Lockless variants of basic read/write operations. Used to orchestrate more
   * complex operations which require the same lock throughout.
   */
  void clear_no_lock() noexcept;
  void write_no_lock(const std::string &data,
                     const std::size_t &count) noexcept;
  std::string read_no_lock(const std::size_t &pos,
                           const std::size_t &count) noexcept;

  std::shared_mutex rw_mutex;

  std::filesystem::path path;
  std::fstream fstream;
};

class Directory final {
public:
  Directory(const Directory &other) = delete;
  Directory &operator=(const Directory &other) noexcept = delete;

  // Iterator supoprt goes here

  /**
   * Opens the file at the given location relative to this directory. Throws
   * exception if no such file exists.
   */
  File &get_file(const std::string &name);

  /**
   * Opens the directory at the given location relative to this directory.
   * Throws an exception if no such directory exists.
   */
  Directory &get_directory(const std::string &name);

  /**
   * Opens the file at the given location relative to this directory, or creates
   * a new file if no file exists at that location.
   */
  File &open_file(const std::string &name) noexcept;

  /**
   * Opens the directory with the given name, or creates a new directory if no
   * directory with a matching name exists.
   */
  Directory &open_directory(const std::string &name) noexcept;

  void create_file(const std::string &name) noexcept;
  void create_directory(const std::string &name) noexcept;

  std::string get_name() const noexcept;
  std::filesystem::path get_path() const noexcept;
  Directory &get_parent() const;

private:
  friend FileSystem;
  Directory(const std::filesystem::path &path, Directory *parent) noexcept;

  bool file_exists(const std::string &name) const noexcept;
  bool directory_exists(const std::string &name) const noexcept;
  bool file_is_cached(const std::string &name) noexcept;
  bool directory_is_cached(const std::string &name) noexcept;

  void cache_file(const std::string &name) noexcept;
  void cache_directory(const std::string &name) noexcept;

  std::shared_mutex rw_mutex;

  Directory *parent;
  std::filesystem::path path;
  std::unordered_map<std::string, std::unique_ptr<File>> cached_files;
  std::unordered_map<std::string, std::unique_ptr<Directory>>
      cached_directories;
};

/**
 * Represents the root of a filesystem. A filesystem is held by each engine for
 * safe concurrent access of files and directories.
 * @note Filesystem operations are only thread safe when performed through the
 * same FileSystem object.
 * @note If you instantiate your own FileSystems, take care to avoid dangling
 * references to Directories/Files.
 */
class FileSystem final {
public:
  explicit FileSystem(const std::filesystem::path &path) noexcept;

  FileSystem(const FileSystem &) = delete;
  FileSystem operator=(const FileSystem &) = delete;

  /**
   * Opens a file, creating a new file if necessary.
   * Throws an exception if file cannot be created/opened.
   * Blocks until write access is available.
   */
  File &open_file(const std::filesystem::path &path);
  Directory &open_directory(const std::filesystem::path &path);

  Directory &get_root_directory() noexcept;

private:
  Directory root;
};
} // namespace junco
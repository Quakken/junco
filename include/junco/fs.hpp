/**
 * @file junco/fs.hpp
 *
 * Declares junco's filesystem utilities and objects, including Files and
 * Directories.
 */

#pragma once

#include <atomic>        // std::atomic_bool
#include <filesystem>    // std::filesystem
#include <fstream>       // std::fstream, std::ifstream
#include <string>        // std::string
#include <unordered_map> // std::unordered_map

#include "junco/exception.hpp" // junco::Exception

namespace junco {

class WriteLock;
class ReadLock;
class DirectoryEntry;

class FileSystemException : public Exception {
public:
  FileSystemException(const char *what = "FileSystem exception") noexcept
      : Exception(what) {}
};

class FileNotFoundException : public FileSystemException {
public:
  FileNotFoundException(const char *what = "FileNotFound exception") noexcept
      : FileSystemException(what) {}
};

/**
 * Object that helps orchestrate safe reads/writes to concurrently accessed
 * data.
 * At any point in time, a ReadWriteGuard can have either...
 *   - one writer, and no readers
 *   - no writers, and any number of readers
 */
class ReadWriteGuard {
public:
public:
  /**
   * Blocks the current thread until a write lock for the guarded resource can
   * be retrieved.
   */
  WriteLock get_write_lock() noexcept;
  /**
   * Blocks the current thread until a read lock for the guarded resource can be
   * retrieved.
   */
  ReadLock get_read_lock() noexcept;

  bool is_writing() const noexcept;
  bool is_reading() const noexcept;

private:
  friend ReadLock;
  friend WriteLock;

  /**
   * Blocks until the resource is in a safe state to write to (no
   * readers/writers).
   */
  void await_write() const noexcept;
  /**
   * Blocks until the resource is in a safe state to read from (no writers)
   */
  void await_read() const noexcept;

  std::atomic_bool writing;
  std::atomic_int reading;
};

/**
 * Holds read access to a ReadWriteGuard until destroyed.
 */
class [[nodiscard("ReadLock uses RAII to release read access on guard.")]]
ReadLock {
public:
  ReadLock(ReadWriteGuard &guard) noexcept : rw_guard(guard) {
    rw_guard.reading++;
  }
  ~ReadLock();
  // TODO: Move constructor?

  ReadLock(const ReadLock &) = delete;
  ReadLock &operator=(const ReadLock &) = delete;

private:
  ReadWriteGuard &rw_guard;
};

/**
 * Holds write access to a ReadWriteGuard until destroyed.
 */
class [[nodiscard("WriteLock uses RAII to release write access on guard.")]]
WriteLock {
public:
  WriteLock(ReadWriteGuard &guard) noexcept : rw_guard(guard) {
    rw_guard.writing = true;
  }
  ~WriteLock();
  // TODO: Move constructor?

  WriteLock(const WriteLock &) = delete;
  WriteLock &operator=(const WriteLock &) = delete;

private:
  ReadWriteGuard &rw_guard;
};

/**
 * Wraps an std::fstream object for thread-safe file I/O.
 */
class File final {
public:
  File(const std::filesystem::path &path) noexcept;
  ~File() = default;

  // Files in junco are always passed by reference, never by value.
  File(const File &) = delete;
  File &operator=(const File &) = delete;

  // TODO: Figure out how to manage this operation with internal file state.
  // This should be a read operation, but since fstream tracks next token,
  // it is actually a write op.
  template <typename T> File &operator>>(T &obj) noexcept {
    auto lock = rw_guard.get_write_lock();
    wrapped >> obj;
    return *this;
  }
  template <typename T> File &operator<<(T &obj) noexcept {
    auto lock = rw_guard.get_write_lock();
    wrapped << obj;
    return *this;
  }

  /**
   * Retrieves the entire contents of the file.
   */
  std::string get_contents() const noexcept;
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

  void set_name(const std::string &new_name) noexcept;
  std::string get_name() const noexcept;
  std::string get_extension() const noexcept;
  std::string get_stem() const noexcept;
  std::filesystem::path get_path() const noexcept;
  std::size_t get_size() const noexcept;

private:
  ReadWriteGuard rw_guard;

  std::filesystem::path path;
  std::fstream wrapped;
};

class Directory final {
public:
  Directory(const std::filesystem::path &path) noexcept;

  Directory(const Directory &other) = delete;
  Directory &operator=(const Directory &other) noexcept = delete;

  // Iterator supoprt goes here

  /**
   * Opens the file at the given location relative to this directory. Throws
   * exception if no such file exists.
   */
  File &get_file(const std::filesystem::path &path);
  const File &get_file(const std::filesystem::path &path) const;

  /**
   * Opens the directory at the given location relative to this directory.
   * Throws an exception if no such directory exists.
   */
  Directory &get_directory(const std::filesystem::path &path);
  const Directory &get_directory(const std::filesystem::path &path) const;

  /**
   * Opens the file at the given location relative to this directory, or creates
   * a new file if no file exists at that location.
   */
  File &open_file(const std::filesystem::path &path) noexcept;

  /**
   * Opens the directory with the given name, or creates a new directory if no
   * directory with a matching name exists.
   */
  Directory &open_directory(const std::filesystem::path &path) noexcept;

private:
  bool entry_exists(const std::filesystem::path &path) noexcept;
  void try_cache_file(const std::filesystem::path &path) noexcept;
  void try_cache_directory(const std::filesystem::path &path) noexcept;

  ReadWriteGuard rw_guard;

  std::filesystem::path path;
  std::unordered_map<std::string, File> cached_files;
  std::unordered_map<std::string, Directory> cached_directories;
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
  FileSystem(const std::filesystem::path &path) noexcept;
  ~FileSystem() = default;

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
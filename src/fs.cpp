#include "junco/fs.hpp"
#include <filesystem> // std::filesystem
#include <mutex>
#include <sstream> // std::stringstream

namespace junco {
File::File(const std::filesystem::path &_path) noexcept
    : path(_path), fstream(path, std::ios::in | std::ios::out | std::ios::ate) {
}

std::string File::get_contents() noexcept { return read(0, get_size()); }
std::string File::read(const std::size_t &pos,
                       const std::size_t &count) noexcept {
  auto buff = std::string(count, '\0');
  {
    // This has to be a unique lock because of seekg.
    auto lock = std::unique_lock(rw_mutex);
    fstream.clear();
    fstream.seekg(pos, std::ios::beg);
    fstream.read(&buff[0], count);
  }
  return buff;
}

void File::set_contents(const std::string &new_contents) noexcept {
  clear();
  write(new_contents, 0);
}
void File::clear() noexcept {
  auto lock = std::unique_lock(rw_mutex);
  // Re-open filestream with truncate flag to clear contents
  fstream.close();
  fstream.open(path, std::ios::in | std::ios::out | std::ios::trunc);
}
void File::append(const std::string &data) noexcept { write(data, get_size()); }
void File::write(const std::string &data, const std::size_t &pos) noexcept {
  auto lock = std::unique_lock(rw_mutex);
  fstream.clear();
  fstream.seekp(pos, std::ios::beg);
  fstream.write(data.c_str(), data.size());
  fstream.flush(); // <-- This is necessary.
}

void File::set_name(const std::string &new_name) noexcept {
  auto new_path = path.replace_filename(new_name);
  std::filesystem::rename(path, new_path);
  path = new_path;
}
std::string File::get_name() const noexcept {
  auto filename = path.filename();
  return filename.string();
}
std::string File::get_extension() const noexcept {
  auto extension = path.extension();
  return extension.string();
}
std::string File::get_stem() const noexcept {
  auto stem = path.stem();
  return stem.string();
}
std::filesystem::path File::get_path() const noexcept { return path; }
std::size_t File::get_size() const noexcept {
  auto ec = std::error_code{};
  // TODO: Error handling here? (SHOULD never happen)
  return std::filesystem::file_size(path, ec);
}

Directory::Directory(const std::filesystem::path &_path,
                     Directory *_parent) noexcept
    : parent(_parent), path(_path) {}

File &Directory::get_file(const std::string &name) {
  if (!file_exists(name))
    throw InvalidPathException("get_file() given name of non-file object");
  if (!file_is_cached(name))
    cache_file(name);
  auto lock = std::shared_lock(rw_mutex);
  return *cached_files.at(name);
}
Directory &Directory::get_directory(const std::string &name) {
  if (!directory_exists(name))
    throw InvalidPathException(
        ("get_directory() given name of non-directory object"));
  if (!directory_is_cached(name))
    cache_directory(name);
  auto lock = std::shared_lock(rw_mutex);
  return *cached_directories.at(name);
}
File &Directory::open_file(const std::string &name) noexcept {
  if (!file_exists(name))
    create_file(name);
  return get_file(name);
}
Directory &Directory::open_directory(const std::string &name) noexcept {
  if (!directory_exists(name))
    create_directory(name);
  return get_directory(name);
}

std::filesystem::path Directory::get_path() const noexcept { return path; }
Directory &Directory::get_parent() const {
  if (!parent)
    throw InvalidPathException("Directory does not have a parent");
  return *parent;
}

bool Directory::file_is_cached(const std::string &name) noexcept {
  auto lock = std::shared_lock(rw_mutex);
  return cached_files.find(name) != cached_files.end();
}
bool Directory::directory_is_cached(const std::string &name) noexcept {
  auto lock = std::shared_lock(rw_mutex);
  return cached_directories.find(name) != cached_directories.end();
}
bool Directory::file_exists(const std::string &name) const noexcept {
  auto target_path = path / name;
  if (!std::filesystem::exists(target_path))
    return false;
  // This *might* be bad logic
  // TODO: Maybe improve this?
  return !directory_exists(name);
}
bool Directory::directory_exists(const std::string &name) const noexcept {
  auto target_path = path / name;
  if (!std::filesystem::exists(target_path))
    return false;
  auto ec = std::error_code{};
  // TODO: Error handling?
  return std::filesystem::is_directory(target_path, ec);
}

void Directory::cache_file(const std::string &name) noexcept {
  auto target_path = path / name;
  auto file = std::unique_ptr<File>(new File(target_path));
  auto lock = std::unique_lock(rw_mutex);
  cached_files.insert({name, std::move(file)});
}
void Directory::cache_directory(const std::string &name) noexcept {
  auto target_path = path / name;
  auto directory = std::unique_ptr<Directory>(new Directory(target_path, this));
  auto lock = std::unique_lock(rw_mutex);
  cached_directories.insert({name, std::move(directory)});
}

void Directory::create_file(const std::string &name) noexcept {
  auto _ = std::ofstream(path / name);
}
void Directory::create_directory(const std::string &name) noexcept {
  std::filesystem::create_directory(path / name);
}

FileSystem::FileSystem(const std::filesystem::path &path) noexcept
    : root(path, nullptr) {}
File &FileSystem::open_file(const std::filesystem::path &path) {
  if (!path.is_relative())
    throw InvalidPathException("open_file() requires a relative path");
  // Descend through each directory in the path until target file is reached
  Directory *current_dir = &root;
  for (auto it = path.begin(); it != path.end(); ++it) {
    auto entry_name = it->string();
    if (it == --path.end())
      return current_dir->open_file(entry_name);
    current_dir = &current_dir->get_directory(entry_name);
  }
  throw FileNotFoundException("Could not find file at the given location");
}
Directory &FileSystem::open_directory(const std::filesystem::path &path) {
  if (!path.is_relative())
    throw InvalidPathException("open_directory() requires a relative path");
  // Descend through each directory in the path until target file is reached
  Directory *current_dir = &root;
  for (auto it = path.begin(); it != path.end(); ++it) {
    auto entry_name = it->string();
    if (it == --path.end())
      return current_dir->open_directory(entry_name);
    if (entry_name == "." || entry_name.empty())
      continue;
    if (entry_name == "..")
      current_dir = &current_dir->get_parent();
    else
      current_dir = &current_dir->get_directory(entry_name);
  }
  throw FileNotFoundException("Could not find file at the given location");
}
Directory &FileSystem::get_root_directory() noexcept { return root; }

} // namespace junco

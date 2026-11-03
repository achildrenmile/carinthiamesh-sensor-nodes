#pragma once
// Dateisystem im Speicher (ersetzt LittleFS/SPIFFS/InternalFS).
#include "Arduino.h"
#include <map>
#include <memory>
#include <vector>

#define FILE_O_READ "r"
#define FILE_O_WRITE "w"

namespace fs {

class File : public Stream {
public:
  struct Data { std::vector<uint8_t> bytes; };
private:
  std::shared_ptr<Data> d;
  size_t pos = 0;
  bool writable = false;
  bool valid = false;
public:
  File() {}
  File(std::shared_ptr<Data> dd, bool w, bool append = false) : d(dd), writable(w), valid(true) { if (append) pos = d->bytes.size(); }
  operator bool() const { return valid; }
  int available() override { return valid ? (int)(d->bytes.size() - pos) : 0; }
  int read() override { return (valid && pos < d->bytes.size()) ? d->bytes[pos++] : -1; }
  int peek() override { return (valid && pos < d->bytes.size()) ? d->bytes[pos] : -1; }
  size_t read(uint8_t* b, size_t n) {
    if (!valid) return 0;
    size_t k = std::min(n, d->bytes.size() - pos);
    memcpy(b, d->bytes.data() + pos, k); pos += k; return k;
  }
  size_t readBytes(char* b, size_t n) override { return read((uint8_t*)b, n); }
  size_t readBytes(uint8_t* b, size_t n) override { return read(b, n); }
  size_t write(uint8_t c) override { return write(&c, 1); }
  size_t write(const uint8_t* b, size_t n) override {
    if (!valid || !writable) return 0;
    if (pos + n > d->bytes.size()) d->bytes.resize(pos + n);
    memcpy(d->bytes.data() + pos, b, n); pos += n; return n;
  }
  size_t size() const { return valid ? d->bytes.size() : 0; }
  size_t position() const { return pos; }
  bool seek(size_t p) { if (!valid || p > d->bytes.size()) return false; pos = p; return true; }
  void close() { valid = false; }
  friend class FS;
};

class FS {
  std::map<std::string, std::shared_ptr<File::Data>> files;
public:
  bool begin(bool = false) { return true; }
  bool format() { files.clear(); return true; }
  bool exists(const char* n) { return files.count(n) > 0; }
  bool remove(const char* n) { return files.erase(n) > 0; }
  bool mkdir(const char*) { return true; }
  bool rename(const char* a, const char* b) { auto it = files.find(a); if (it == files.end()) return false; files[b] = it->second; files.erase(it); return true; }
  File open(const char* n, const char* mode = "r", bool = false) {
    bool w = mode && (mode[0] == 'w' || mode[0] == 'a');
    auto it = files.find(n);
    if (!w && it == files.end()) return File();
    if (w && (mode[0] == 'w' || it == files.end())) files[n] = std::make_shared<File::Data>();
    return File(files[n], w, mode && mode[0] == 'a');
  }
  size_t totalBytes() { return 1 << 20; }
  size_t usedBytes() { size_t t = 0; for (auto& f : files) t += f.second->bytes.size(); return t; }
};
}  // namespace fs

using fs::File;
typedef fs::FS SimFS;

inline fs::FS SPIFFS;
inline fs::FS LittleFS;
inline fs::FS InternalFS;

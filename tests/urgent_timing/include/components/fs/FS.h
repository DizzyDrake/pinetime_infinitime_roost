#pragma once
#include <cstdint>
struct lfs_file_t {};
constexpr int LFS_ERR_OK=0;
constexpr int LFS_O_RDONLY=1;
constexpr int LFS_O_WRONLY=2;
constexpr int LFS_O_CREAT=4;
constexpr int LFS_O_TRUNC=8;
namespace Pinetime { namespace Controllers {
class FS {
public:
  int FileOpen(lfs_file_t*, const char*, int) { return -1; }
  int FileRead(lfs_file_t*, uint8_t*, unsigned) { return -1; }
  int FileWrite(lfs_file_t*, const uint8_t*, unsigned) { return -1; }
  int FileClose(lfs_file_t*) { return LFS_ERR_OK; }
};
}}

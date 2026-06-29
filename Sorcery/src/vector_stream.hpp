#pragma once

#include <cstddef>
#include <istream>
#include <ostream>
#include <span>
#include <streambuf>
#include <vector>


namespace sorcery {
class ByteVectorStreambuf : public std::basic_streambuf<char> {
public:
  explicit ByteVectorStreambuf(std::vector<std::byte>& buffer);

protected:
  auto overflow(int_type ch) -> int_type override;
  auto xsputn(char const* s, std::streamsize n) -> std::streamsize override;
  auto seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which) -> pos_type override;
  auto seekpos(pos_type pos, std::ios_base::openmode which) -> pos_type override;

private:
  auto WriteAtCurrentPosition(char const* s, std::size_t n) -> void;

  std::vector<std::byte>& buffer_;
  std::size_t pos_ = 0;
};


class ByteVectorOstream : public std::basic_ostream<char> {
public:
  explicit ByteVectorOstream(std::vector<std::byte>& buffer);

private:
  ByteVectorStreambuf buf_;
};


class ByteSpanStreambuf : public std::basic_streambuf<char> {
public:
  explicit ByteSpanStreambuf(std::span<std::byte const> bytes);

protected:
  auto underflow() -> int_type override;
  auto uflow() -> int_type override;
  auto xsgetn(char* s, std::streamsize n) -> std::streamsize override;
  auto showmanyc() -> std::streamsize override;
  auto seekoff(off_type off, std::ios_base::seekdir dir, std::ios_base::openmode which) -> pos_type override;
  auto seekpos(pos_type pos, std::ios_base::openmode which) -> pos_type override;

private:
  std::span<std::byte const> bytes_;
  std::size_t pos_ = 0;
};


class ByteSpanIstream : public std::basic_istream<char> {
public:
  explicit ByteSpanIstream(std::span<std::byte const> bytes);

private:
  ByteSpanStreambuf buf_;
};
}

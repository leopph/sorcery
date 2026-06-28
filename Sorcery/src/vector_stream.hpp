#pragma once

#include <algorithm>
#include <cstddef>
#include <cstring>
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

private:
  std::vector<std::byte>& buffer_;
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

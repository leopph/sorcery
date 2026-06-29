#include "vector_stream.hpp"

#include <utility>


namespace sorcery {
ByteVectorStreambuf::ByteVectorStreambuf(std::vector<std::byte>& buffer) :
  buffer_{buffer} {}


auto ByteVectorStreambuf::overflow(int_type const ch) -> std::streambuf::int_type {
  if (traits_type::eq_int_type(ch, traits_type::eof())) {
    return traits_type::not_eof(ch);
  }

  char const c = traits_type::to_char_type(ch);
  WriteAtCurrentPosition(&c, 1);

  return ch;
}


auto ByteVectorStreambuf::xsputn(char const* s, std::streamsize const n) -> std::streamsize {
  if (n <= 0) {
    return 0;
  }

  WriteAtCurrentPosition(s, static_cast<std::size_t>(n));
  return n;
}


auto ByteVectorStreambuf::seekoff(off_type const off, std::ios_base::seekdir const dir,
                                  std::ios_base::openmode const which) -> std::streambuf::pos_type {
  if ((which & std::ios_base::out) == 0) {
    return {static_cast<off_type>(-1)};
  }

  std::size_t base;

  switch (dir) {
    case std::ios_base::beg:
      base = 0;
      break;

    case std::ios_base::cur:
      base = pos_;
      break;

    case std::ios_base::end:
      base = buffer_.size();
      break;

    default:
      return {static_cast<off_type>(-1)};
  }

  if (off < 0 && std::cmp_greater(-off, base)) {
    return {-1};
  }

  auto const new_pos = static_cast<std::size_t>(
    static_cast<off_type>(base) + off
  );

  pos_ = new_pos;
  return {static_cast<off_type>(pos_)};
}


auto ByteVectorStreambuf::seekpos(pos_type const pos, std::ios_base::openmode const which) -> std::streambuf::pos_type {
  if ((which & std::ios_base::out) == 0) {
    return {static_cast<off_type>(-1)};
  }

  auto const off = static_cast<off_type>(pos);

  if (off < 0) {
    return {static_cast<off_type>(-1)};
  }

  pos_ = static_cast<std::size_t>(off);
  return pos;
}


auto ByteVectorStreambuf::WriteAtCurrentPosition(char const* s, std::size_t n) -> void {
  auto const required_size = pos_ + n;

  if (required_size > buffer_.size()) {
    buffer_.resize(required_size);
  }

  std::memcpy(buffer_.data() + pos_, s, n);
  pos_ += n;
}


ByteVectorOstream::ByteVectorOstream(std::vector<std::byte>& buffer) :
  std::basic_ostream<char>{nullptr},
  buf_{buffer} {
  this->rdbuf(&buf_);
}


ByteSpanStreambuf::ByteSpanStreambuf(std::span<std::byte const> const bytes) :
  bytes_{bytes} {}


auto ByteSpanStreambuf::underflow() -> std::streambuf::int_type {
  if (pos_ >= bytes_.size()) {
    return traits_type::eof();
  }

  auto const byte = std::to_integer<unsigned char>(bytes_[pos_]);
  auto const c = static_cast<char>(byte);

  return traits_type::to_int_type(c);
}


auto ByteSpanStreambuf::uflow() -> std::streambuf::int_type {
  auto const result = underflow();

  if (!traits_type::eq_int_type(result, traits_type::eof())) {
    ++pos_;
  }

  return result;
}


auto ByteSpanStreambuf::xsgetn(char* s, std::streamsize const n) -> std::streamsize {
  if (n <= 0) {
    return 0;
  }

  auto const remaining = bytes_.size() - pos_;
  auto const requested = static_cast<std::size_t>(n);
  auto const count = std::min(remaining, requested);

  std::memcpy(s, bytes_.data() + pos_, count);
  pos_ += count;

  return static_cast<std::streamsize>(count);
}


auto ByteSpanStreambuf::showmanyc() -> std::streamsize {
  return static_cast<std::streamsize>(bytes_.size() - pos_);
}


auto ByteSpanStreambuf::seekoff(off_type const off, std::ios_base::seekdir const dir,
                                std::ios_base::openmode const which) -> std::streambuf::pos_type {
  if ((which & std::ios_base::in) == 0) {
    return {static_cast<off_type>(-1)};
  }

  std::size_t base;

  switch (dir) {
    case std::ios_base::beg:
      base = 0;
      break;

    case std::ios_base::cur:
      base = pos_;
      break;

    case std::ios_base::end:
      base = bytes_.size();
      break;

    default:
      return {static_cast<off_type>(-1)};
  }

  if (off < 0 && std::cmp_greater(-off, base)) {
    return {static_cast<off_type>(-1)};
  }

  auto const new_pos = static_cast<std::size_t>(
    static_cast<off_type>(base) + off
  );

  if (new_pos > bytes_.size()) {
    return {static_cast<off_type>(-1)};
  }

  pos_ = new_pos;
  return {static_cast<off_type>(pos_)};
}


auto ByteSpanStreambuf::seekpos(pos_type const pos, std::ios_base::openmode const which) -> std::streambuf::pos_type {
  if ((which & std::ios_base::in) == 0) {
    return {static_cast<off_type>(-1)};
  }

  auto const off = static_cast<off_type>(pos);

  if (off < 0) {
    return {static_cast<off_type>(-1)};
  }

  auto const new_pos = static_cast<std::size_t>(off);

  if (new_pos > bytes_.size()) {
    return {static_cast<off_type>(-1)};
  }

  pos_ = new_pos;
  return pos;
}


ByteSpanIstream::ByteSpanIstream(std::span<std::byte const> const bytes) :
  std::basic_istream<char>{nullptr},
  buf_{bytes} {
  this->rdbuf(&buf_);
}
}

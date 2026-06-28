#include "vector_stream.hpp"


namespace sorcery {
ByteVectorStreambuf::ByteVectorStreambuf(std::vector<std::byte>& buffer) :
  buffer_(buffer) {}


auto ByteVectorStreambuf::overflow(int_type const ch) -> std::streambuf::int_type {
  if (traits_type::eq_int_type(ch, traits_type::eof())) {
    return traits_type::not_eof(ch);
  }

  char const c = traits_type::to_char_type(ch);
  buffer_.push_back(static_cast<std::byte>(static_cast<unsigned char>(c)));

  return ch;
}


auto ByteVectorStreambuf::xsputn(char const* s, std::streamsize const n) -> std::streamsize {
  if (n <= 0) {
    return 0;
  }

  auto const old_size = buffer_.size();
  auto const count = static_cast<std::size_t>(n);

  buffer_.resize(old_size + count);
  std::memcpy(buffer_.data() + old_size, s, count);

  return n;
}


ByteVectorOstream::ByteVectorOstream(std::vector<std::byte>& buffer) :
  std::basic_ostream<char>{nullptr},
  buf_(buffer) {
  this->rdbuf(&buf_);
}


ByteSpanStreambuf::ByteSpanStreambuf(std::span<std::byte const> const bytes) :
  bytes_(bytes) {}


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


ByteSpanIstream::ByteSpanIstream(std::span<std::byte const> bytes) :
  std::basic_istream<char>{nullptr},
  buf_(bytes) {
  this->rdbuf(&buf_);
}
}

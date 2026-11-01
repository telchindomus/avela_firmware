#include "avela_radio/stream/icy_metadata_reader.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace avela {
namespace {

using Bytes = std::vector<uint8_t>;

void append(Bytes& bytes, std::string_view text)
{
    bytes.insert(bytes.end(), text.begin(), text.end());
}

// A metadata block as a station sends it: the length byte (16-byte units), the text, zero padding.
void append_block(Bytes& bytes, std::string_view text)
{
    const std::size_t units = (text.size() + 15) / 16;
    bytes.push_back(static_cast<uint8_t>(units));
    append(bytes, text);
    bytes.insert(bytes.end(), (units * 16) - text.size(), uint8_t{0});
}

std::string as_text(std::span<const uint8_t> bytes)
{
    return {bytes.begin(), bytes.end()};
}

// Feeds bytes in pieces of piece_size and collects the audio.
std::string strip_in_pieces(IcyMetadataReader& reader, Bytes bytes, std::size_t piece_size)
{
    std::string audio;
    for (std::size_t start = 0; start < bytes.size(); start += piece_size) {
        const std::size_t size = std::min(piece_size, bytes.size() - start);
        audio += as_text(reader.strip(std::span{bytes}.subspan(start, size)));
    }
    return audio;
}

// Audio every 4 bytes: "AAAA", a title block, "BBBB", an empty block, "CC".
Bytes stream_with_title(std::string_view metadata)
{
    Bytes bytes;
    append(bytes, "AAAA");
    append_block(bytes, metadata);
    append(bytes, "BBBB");
    bytes.push_back(0);
    append(bytes, "CC");
    return bytes;
}

TEST_CASE("without a metadata interval every byte is audio")
{
    IcyMetadataReader reader{0};
    Bytes bytes;
    append(bytes, "AB\0CD");
    CHECK(as_text(reader.strip(bytes)) == as_text(bytes));
    CHECK(reader.title().empty());
}

TEST_CASE("the metadata blocks are taken out and the title kept")
{
    IcyMetadataReader reader{4};
    CHECK(strip_in_pieces(reader, stream_with_title("StreamTitle='Bonobo - Kerala';"), 1000) == "AAAABBBBCC");
    CHECK(reader.title() == "Bonobo - Kerala");
    CHECK(reader.title_revision() == 1);
}

TEST_CASE("blocks split across reads anywhere give the same result")
{
    const Bytes bytes = stream_with_title("StreamTitle='Bonobo - Kerala';StreamUrl='';");
    for (std::size_t piece_size = 1; piece_size <= bytes.size(); ++piece_size) {
        IcyMetadataReader reader{4};
        CAPTURE(piece_size);
        CHECK(strip_in_pieces(reader, bytes, piece_size) == "AAAABBBBCC");
        CHECK(reader.title() == "Bonobo - Kerala");
    }
}

TEST_CASE("a title may hold apostrophes: it ends at the first \"';\"")
{
    IcyMetadataReader reader{4};
    strip_in_pieces(reader, stream_with_title("StreamTitle='Guns N' Roses - Patience';StreamUrl='x';"), 1000);
    CHECK(reader.title() == "Guns N' Roses - Patience");
}

TEST_CASE("the same title again doesn't count as a change")
{
    Bytes bytes;
    append(bytes, "AAAA");
    append_block(bytes, "StreamTitle='One';");
    append(bytes, "BBBB");
    append_block(bytes, "StreamTitle='One';");
    IcyMetadataReader reader{4};
    strip_in_pieces(reader, bytes, 1000);
    CHECK(reader.title() == "One");
    CHECK(reader.title_revision() == 1);
}

TEST_CASE("a block without a title leaves the title as it was")
{
    IcyMetadataReader reader{4};
    Bytes bytes;
    append(bytes, "AAAA");
    append_block(bytes, "StreamTitle='One';");
    append(bytes, "BBBB");
    append_block(bytes, "StreamUrl='http://example.com';");
    CHECK(strip_in_pieces(reader, bytes, 1000) == "AAAABBBB");
    CHECK(reader.title() == "One");
}

TEST_CASE("a long title is cut at a whole UTF-8 character")
{
    const std::string title = std::string(IcyMetadataReader::title_capacity - 2, 'a') + "\xC3\xA9";  // ... "é"
    IcyMetadataReader reader{4};
    strip_in_pieces(reader, stream_with_title("StreamTitle='" + title + "';"), 1000);
    CHECK(reader.title() == std::string(IcyMetadataReader::title_capacity - 2, 'a'));
}

}  // namespace
}  // namespace avela

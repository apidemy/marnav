#include "marnav/nmea/nrx.hpp"
#include "type_traits_helper.hpp"
#include "marnav/nmea/nmea.hpp"
#include <gtest/gtest.h>

namespace
{
using namespace marnav;

class test_nmea_nrx : public ::testing::Test
{
};

TEST_F(test_nmea_nrx, contruction)
{
	EXPECT_NO_THROW(nmea::nrx nrx);
}

TEST_F(test_nmea_nrx, properties)
{
	nmea_sentence_traits<nmea::nrx>();
}

TEST_F(test_nmea_nrx, parse)
{
	std::string_view msg
		= "$CRNRX,007,001,00,IE69,1,135600,27,06,2001,241,3,A,==========================*09";

	auto s = nmea::make_sentence(msg.data());
	ASSERT_NE(nullptr, s);

	auto nrx = nmea::sentence_cast<nmea::nrx>(s);
	ASSERT_NE(nullptr, nrx);

	ASSERT_EQ(msg.data(), nmea::to_string(*nrx));
}

TEST_F(test_nmea_nrx, parse_invalid_number_of_arguments)
{
	EXPECT_ANY_THROW(
		nmea::detail::factory::sentence_parse<nmea::nrx>(nmea::talker::none, {12, "@"}));
	EXPECT_ANY_THROW(
		nmea::detail::factory::sentence_parse<nmea::nrx>(nmea::talker::none, {14, "@"}));
}

TEST_F(test_nmea_nrx, empty_to_string)
{
	nmea::nrx nrx;

	EXPECT_STREQ("$CRNRX,000,000,00,,,,,,,,,,*79", nmea::to_string(nrx).c_str());
}

TEST_F(test_nmea_nrx, set_number_sentences)
{
	nmea::nrx nrx;
	nrx.set_number_sentences(999);

	EXPECT_STREQ("$CRNRX,999,000,00,,,,,,,,,,*70", nmea::to_string(nrx).c_str());
}

TEST_F(test_nmea_nrx, set_sentence_number)
{
	nmea::nrx nrx;
	nrx.set_sentence_number(66);

	EXPECT_STREQ("$CRNRX,000,066,00,,,,,,,,,,*79", nmea::to_string(nrx).c_str());
}

TEST_F(test_nmea_nrx, set_sequential_id)
{
	nmea::nrx nrx;
	nrx.set_sequential_id(10);

	EXPECT_STREQ("$CRNRX,000,000,10,,,,,,,,,,*78", nmea::to_string(nrx).c_str());
}

TEST_F(test_nmea_nrx, set_message_code)
{
	nmea::nrx nrx;
	nrx.set_message_code({'T', 'D', 6});

	EXPECT_STREQ("$CRNRX,000,000,00,TD06,,,,,,,,,*6F", nmea::to_string(nrx).c_str());
}

TEST_F(test_nmea_nrx, set_frequency_index)
{
	nmea::nrx nrx;
	nrx.set_frequency_index(nmea::frequency_index::F_518kHz);

	EXPECT_STREQ("$CRNRX,000,000,00,,2,,,,,,,,*4B", nmea::to_string(nrx).c_str());
}

TEST_F(test_nmea_nrx, set_time_utc)
{
	nmea::nrx nrx;
	nrx.set_time_utc(nmea::time{14, 57, 6});

	EXPECT_STREQ("$CRNRX,000,000,00,,,145706,,,,,,,*78", nmea::to_string(nrx).c_str());
}

TEST_F(test_nmea_nrx, set_date)
{
	nmea::nrx nrx;
	nrx.set_date(nmea::date{2023, nmea::month::april, 7});

	EXPECT_STREQ("$CRNRX,000,000,00,,,,07,04,2023,,,,*79", nmea::to_string(nrx).c_str());
}

TEST_F(test_nmea_nrx, set_total_characters)
{
	nmea::nrx nrx;
	nrx.set_total_characters(156);

	EXPECT_STREQ("$CRNRX,000,000,00,,,,,,,156,,,*4B", nmea::to_string(nrx).c_str());
}

TEST_F(test_nmea_nrx, set_total_bad_characters)
{
	nmea::nrx nrx;
	nrx.set_total_bad_characters(61);

	EXPECT_STREQ("$CRNRX,000,000,00,,,,,,,,61,,*7E", nmea::to_string(nrx).c_str());
}

TEST_F(test_nmea_nrx, set_status)
{
	nmea::nrx nrx;
	nrx.set_status(nmea::status::ok);

	EXPECT_STREQ("$CRNRX,000,000,00,,,,,,,,,A,*38", nmea::to_string(nrx).c_str());
}

TEST_F(test_nmea_nrx, set_message)
{
	nmea::nrx nrx;
	nrx.set_message("WINDS^2C STRONGEST IN NORTH.^0D");

	EXPECT_STREQ("$CRNRX,000,000,00,,,,,,,,,,WINDS^2C STRONGEST IN NORTH.^0D*2C",
		nmea::to_string(nrx).c_str());
}
}
#include <gtest/gtest.h>
#include <nmea/gga.hpp>
#include <nmea/nmea.hpp>

namespace
{

class Test_nmea_gga : public ::testing::Test
{
};

TEST_F(Test_nmea_gga, contruction)
{
	nmea::gga gga;
}

TEST_F(Test_nmea_gga, size)
{
	EXPECT_EQ(192u, sizeof(nmea::gga));
}

TEST_F(Test_nmea_gga, parse)
{
	auto s = nmea::make_sentence("$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47");
	ASSERT_NE(nullptr, s);

	auto gga = nmea::sentence_cast<nmea::gga>(s);
	ASSERT_NE(nullptr, gga);
}

TEST_F(Test_nmea_gga, parse_invalid_number_of_arguments)
{
	EXPECT_ANY_THROW(nmea::gga::parse("@@", {13, "@"}));
	EXPECT_ANY_THROW(nmea::gga::parse("@@", {15, "@"}));
}

TEST_F(Test_nmea_gga, empty_to_string)
{
	nmea::gga gga;

	EXPECT_STREQ("$GPGGA,,,,,,,,,,,,,,*56", nmea::to_string(gga).c_str());
}

TEST_F(Test_nmea_gga, set_time)
{
	nmea::gga gga;
	gga.set_time(nmea::time{12, 34, 56, 0});

	EXPECT_STREQ("$GPGGA,123456,,,,,,,,,,,,,*51", nmea::to_string(gga).c_str());
}

TEST_F(Test_nmea_gga, set_lat_north)
{
	nmea::gga gga;
	gga.set_lat(nmea::latitude{12.345});

	EXPECT_STREQ("$GPGGA,,1220.7000,N,,,,,,,,,,,*30", nmea::to_string(gga).c_str());
}

TEST_F(Test_nmea_gga, set_lat_south)
{
	nmea::gga gga;
	gga.set_lat(nmea::latitude{-12.345});

	EXPECT_STREQ("$GPGGA,,1220.7000,S,,,,,,,,,,,*2D", nmea::to_string(gga).c_str());
}

TEST_F(Test_nmea_gga, set_lon_west)
{
	nmea::gga gga;
	gga.set_lon(nmea::longitude{123.45});

	EXPECT_STREQ("$GPGGA,,,,12327.0000,W,,,,,,,,,*1A", nmea::to_string(gga).c_str());
}

TEST_F(Test_nmea_gga, set_lon_east)
{
	nmea::gga gga;
	gga.set_lon(nmea::longitude{-123.45});

	EXPECT_STREQ("$GPGGA,,,,12327.0000,E,,,,,,,,,*08", nmea::to_string(gga).c_str());
}

}


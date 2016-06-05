#ifndef __NMEA__ZTG__HPP__
#define __NMEA__ZTG__HPP__

#include <marnav/nmea/sentence.hpp>
#include <marnav/nmea/time.hpp>
#include <marnav/utils/optional.hpp>

namespace marnav
{
namespace nmea
{
MARNAV_NMEA_DECLARE_SENTENCE_PARSE_FUNC(ztg)

/// @brief ZTG - UTC & Time to Destination Waypoint
///
/// @code
///        1         2         3
///        |         |         |
/// $--ZTG,hhmmss.ss,hhmmss.ss,c--c*hh<CR><LF>
/// @endcode
///
/// Field Number:
/// 1. Universal Time Coordinated (UTC)
/// 2. Time Remaining
/// 3. Destination Waypoint ID
///
class ztg : public sentence
{
	MARNAV_NMEA_SENTENCE_FRIENDS(ztg)

public:
	constexpr static const sentence_id ID = sentence_id::ZTG;
	constexpr static const char * TAG = "ZTG";

	ztg();
	ztg(const ztg &) = default;
	ztg & operator=(const ztg &) = default;

protected:
	ztg(const std::string & talker, fields::const_iterator first, fields::const_iterator last);
	virtual std::vector<std::string> get_data() const override;

private:
	utils::optional<nmea::time> time_utc;
	utils::optional<nmea::time> time_remaining;
	utils::optional<std::string> waypoint_id;

public:
	NMEA_GETTER(time_utc)
	NMEA_GETTER(time_remaining)
	NMEA_GETTER(waypoint_id)

	void set_time_utc(const nmea::time & t) noexcept { time_utc = t; }
	void set_time_remaining(const nmea::time & t) noexcept { time_remaining = t; }
	void set_waypoint_id(const std::string & id);
};
}
}

#endif

#include "lcd.hpp"
#include <marnav/nmea/io.hpp>

namespace marnav
{
namespace nmea
{
MARNAV_NMEA_DEFINE_SENTENCE_PARSE_FUNC(lcd)

constexpr const char * lcd::TAG;

lcd::lcd()
	: sentence(ID, TAG, talker_id::global_positioning_system)
	, master({0, 0})
{
}

lcd::lcd(const std::string & talker, fields::const_iterator first, fields::const_iterator last)
	: sentence(ID, TAG, talker)
{
	if (std::distance(first, last) != 13)
		throw std::invalid_argument{"invalid number of fields in lcd"};

	read(*(first + 0), gri);
	read(*(first + 1), master.snr);
	read(*(first + 2), master.ecd);
	for (int i = 0; i < num_differences; ++i) {
		utils::optional<decltype(time_difference::snr)> snr;
		utils::optional<decltype(time_difference::ecd)> ecd;
		read(*(first + (i * 2) + 3 + 0), snr);
		read(*(first + (i * 2) + 3 + 1), ecd);
		if (snr && ecd) {
			time_diffs[i] = utils::make_optional<time_difference>(*snr, *ecd);
		}
	}
}

void lcd::check_index(int index) const
{
	if ((index < 0) || (index >= num_differences)) {
		throw std::out_of_range{"time difference index out of range"};
	}
}

utils::optional<lcd::time_difference> lcd::get_time_diff(int index) const
{
	check_index(index);
	return time_diffs[index];
}

void lcd::set_time_diff(int index, time_difference t)
{
	check_index(index);
	time_diffs[index] = t;
}

std::vector<std::string> lcd::get_data() const
{
	std::vector<std::string> result;
	result.reserve(13);
	result.push_back(to_string(gri));
	result.push_back(format(master.snr, 3));
	result.push_back(format(master.ecd, 3));
	for (int i = 0; i < num_differences; ++i) {
		auto const & t = time_diffs[i];
		if (t) {
			result.push_back(format(t->snr, 3));
			result.push_back(format(t->ecd, 3));
		} else {
			result.push_back("");
			result.push_back("");
		}
	}
	return result;
}
}
}

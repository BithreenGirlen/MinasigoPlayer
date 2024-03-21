#ifndef MNSG_H_
#define MNSG_H_

#include <string>
#include <vector>

#include "adv.h"

namespace mnsg
{
	bool LoadScenario(const std::wstring& wstrFilePath, std::vector<adv::TextDatum>& textData);
}
#endif // !MNSG_H_

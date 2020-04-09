#ifndef _MOBILEIMPL_H
#define _MOBILEIMPL_H

#include <iostream>

namespace Firefinch
{

namespace Net
{

class mobileImpl
{

public:
	virtual ~mobileImpl() {}
	virtual bool connect() = 0;
	virtual bool disconnect() = 0;
	virtual std::string getMobileInfo() = 0;
	virtual bool checkNetRegsterStatus() = 0;

private:
};

} // namespace Net

} // namespace Firefinch

#endif

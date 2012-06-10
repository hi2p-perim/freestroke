#include "util.h"

Util::Util()
{

}

void Util::ShowStatusMessage( QString mes )
{
	emit StatusMessage(mes);
}

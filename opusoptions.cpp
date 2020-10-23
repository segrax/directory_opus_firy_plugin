#include "stdafx.h"
#include <codecvt>

cOpusOptions::cOpusOptions() {

}

firy::spOptionResponse cOpusOptions::warning(const std::string& pModule, const std::string& pMessage, const std::string& pMessageDetail) {
	if (!mWarningShow)
		return std::make_shared<firy::cOptionResponse>(true);

	auto msg = s2ws(pMessage);
	msg += s2ws(pMessageDetail);

	auto mod = L"Continue with " + s2ws(pModule);

	SHOWREQUESTDLGDATA data;
	memset(&data, 0, sizeof(data));
	data.cbSize = sizeof(data);
	data.hwndParent = DOpus.GetFunctionWindow(0);
	data.pszTitle = mod.c_str();
	data.pszMessage = msg.c_str();
	data.pszButtons = BUTTONS_OKCANCEL;
	data.hIcon = HICON_WARNING;

	if (!DOpusUtil.ShowRequestDlg(&data)) {
		return std::make_shared<firy::cOptionResponse>( true );
	}

	return std::make_shared<firy::cOptionResponse>(false);

}

void cOpusOptions::error(const std::string& pModule, const std::string& pMessage, const std::string& pMessageDetail) {
	if (!mErrorShow)
		return;

	auto msg = s2ws(pMessage);
	msg += s2ws(pMessageDetail);

	auto mod = L"Aborted in " + s2ws(pModule);

	SHOWREQUESTDLGDATA data;
	memset(&data, 0, sizeof(data));
	data.cbSize = sizeof(data);
	data.hwndParent = DOpus.GetFunctionWindow(0);
	data.pszTitle = mod.c_str();
	data.pszMessage = msg.c_str();
	data.pszButtons = BUTTONS_OK;
	data.hIcon = HICON_ERROR;

	DOpusUtil.ShowRequestDlg(&data);
}
